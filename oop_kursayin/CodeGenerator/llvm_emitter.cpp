#include "llvm_emitter.h"
#include <stdexcept>
 
// ── Utilities ────────────────────────────────────────────────────────────────
 
std::string LLVMEmitter::new_tmp() {
    return "%t" + std::to_string(tmp_++);
}
 
std::string LLVMEmitter::new_label(const std::string& hint) {
    return hint + "_" + std::to_string(lbl_++);
}
 
std::string LLVMEmitter::llvm_type(const AshType& t) {
    if (t.is_fn) return "ptr"; // function pointers are opaque ptr in LLVM 15+
    switch (t.prim) {
        case PrimType::Int:   return "i32";
        case PrimType::Float: return "double";
        case PrimType::Bool:  return "i1";
        case PrimType::Void:  return "void";
    }
    return "i32";
}
 
// ── Program ──────────────────────────────────────────────────────────────────
 
std::string LLVMEmitter::emit(const Program& prog) {
    out_.str(""); out_.clear();
 
    // Target triple for RISC-V 32-bit Linux
    out_ << "; Ash compiler output\n";
    out_ << "target triple = \"riscv32-unknown-linux-gnu\"\n";
    out_ << "target datalayout = \"e-m:e-p:32:32-i64:64-n32-S128\"\n\n";
 
    for (auto& fn : prog.functions) emit_fn(fn);
    return out_.str();
}
 
// ── Function ─────────────────────────────────────────────────────────────────
 
void LLVMEmitter::emit_fn(const FnDecl& fn) {
    tmp_ = 0; lbl_ = 0;
    locals_.clear();
 
    // Signature
    out_ << "define " << llvm_type(fn.ret_type) << " @" << fn.name << "(";
    for (std::size_t i = 0; i < fn.params.size(); ++i) {
        if (i) out_ << ", ";
        out_ << llvm_type(fn.params[i].type) << " %" << fn.params[i].name;
    }
    out_ << ") {\nentry:\n";
 
    // Alloca + store each param so we can SSA-load later
    for (auto& p : fn.params) {
        std::string slot = "%sl_" + p.name;
        out_ << "  " << slot << " = alloca " << llvm_type(p.type) << "\n";
        out_ << "  store " << llvm_type(p.type) << " %" << p.name
             << ", ptr " << slot << "\n";
        locals_[p.name] = slot;
    }
 
    for (auto& s : fn.body) emit_stmt(*s);
 
    // Safety: void functions need a terminal
    if (fn.ret_type.prim == PrimType::Void)
        out_ << "  ret void\n";
 
    out_ << "}\n\n";
}
 
// ── Statements ───────────────────────────────────────────────────────────────
 
void LLVMEmitter::emit_stmt(const Stmt& s) {
    if (auto* let = dynamic_cast<const LetStmt*>(&s)) {
        std::string val  = emit_expr(*let->init);
        std::string slot = "%sl_" + let->name;
        out_ << "  " << slot << " = alloca " << llvm_type(let->declared_type) << "\n";
        out_ << "  store " << llvm_type(let->declared_type) << " " << val
             << ", ptr " << slot << "\n";
        locals_[let->name] = slot;
        return;
    }
    if (auto* ret = dynamic_cast<const ReturnStmt*>(&s)) {
        std::string val = emit_expr(*ret->value);
        out_ << "  ret " << llvm_type(ret->value->type) << " " << val << "\n";
        return;
    }
    if (auto* ifs = dynamic_cast<const IfStmt*>(&s)) {
        std::string cond  = emit_expr(*ifs->cond);
        std::string l_then = new_label("then");
        std::string l_else = new_label("else");
        std::string l_end  = new_label("end");
        out_ << "  br i1 " << cond
             << ", label %" << l_then
             << ", label %" << (ifs->else_body.empty() ? l_end : l_else) << "\n";
        out_ << l_then << ":\n";
        for (auto& st : ifs->then_body) emit_stmt(*st);
        out_ << "  br label %" << l_end << "\n";
        if (!ifs->else_body.empty()) {
            out_ << l_else << ":\n";
            for (auto& st : ifs->else_body) emit_stmt(*st);
            out_ << "  br label %" << l_end << "\n";
        }
        out_ << l_end << ":\n";
        return;
    }
    if (auto* es = dynamic_cast<const ExprStmt*>(&s)) {
        emit_expr(*es->expr);
        return;
    }
    throw std::runtime_error("Unknown stmt in codegen");
}
 
// ── Expressions ──────────────────────────────────────────────────────────────
 
std::string LLVMEmitter::emit_expr(const Expr& e) {
    if (auto* n = dynamic_cast<const IntLitExpr*>(&e)) {
        std::string r = new_tmp();
        out_ << "  " << r << " = add i32 0, " << n->value << "\n";
        return r;
    }
    if (auto* n = dynamic_cast<const FloatLitExpr*>(&e)) {
        std::string r = new_tmp();
        out_ << "  " << r << " = fadd double 0.0, " << n->value << "\n";
        return r;
    }
    if (auto* n = dynamic_cast<const BoolLitExpr*>(&e)) {
        std::string r = new_tmp();
        out_ << "  " << r << " = add i1 0, " << (n->value ? 1 : 0) << "\n";
        return r;
    }
 
    if (auto* n = dynamic_cast<const IdentExpr*>(&e)) {
        // Could be a function name (global) or a local variable
        auto it = locals_.find(n->name);
        if (it == locals_.end()) {
            // It's a function — return its address
            return "@" + n->name;
        }
        std::string r = new_tmp();
        out_ << "  " << r << " = load " << llvm_type(e.type)
             << ", ptr " << it->second << "\n";
        return r;
    }
 
    if (auto* n = dynamic_cast<const BinopExpr*>(&e)) {
        std::string lv = emit_expr(*n->lhs);
        std::string rv = emit_expr(*n->rhs);
        std::string r  = new_tmp();
        bool is_float  = (n->lhs->type.prim == PrimType::Float);
        bool is_cmp    = (n->op == "==" || n->op == "!=" ||
                          n->op == "<"  || n->op == ">"  ||
                          n->op == "<=" || n->op == ">=");
 
        if (n->op == "&&") {
            out_ << "  " << r << " = and i1 " << lv << ", " << rv << "\n";
        } else if (n->op == "||") {
            out_ << "  " << r << " = or i1 " << lv << ", " << rv << "\n";
        } else if (is_cmp) {
            std::string pred;
            if (is_float) {
                static std::unordered_map<std::string,std::string> fp = {
                    {"==","oeq"},{"!=","one"},{"<","olt"},{">","ogt"},{"<=","ole"},{">=","oge"}};
                out_ << "  " << r << " = fcmp " << fp[n->op] << " double " << lv << ", " << rv << "\n";
            } else {
                bool is_bool = (n->lhs->type.prim == PrimType::Bool);
                static std::unordered_map<std::string,std::string> ip = {
                    {"==","eq"},{"!=","ne"},{"<","slt"},{">","sgt"},{"<=","sle"},{">=","sge"}};
                std::string pfx = is_bool ? "i" : "s";
                out_ << "  " << r << " = icmp " << ip[n->op] << " "
                     << llvm_type(n->lhs->type) << " " << lv << ", " << rv << "\n";
            }
        } else {
            // arithmetic
            static std::unordered_map<std::string,std::pair<std::string,std::string>> arith = {
                {"+", {"add",  "fadd"}},
                {"-", {"sub",  "fsub"}},
                {"*", {"mul",  "fmul"}},
                {"/", {"sdiv", "fdiv"}},
            };
            auto [iop, fop] = arith.at(n->op);
            if (is_float)
                out_ << "  " << r << " = " << fop << " double " << lv << ", " << rv << "\n";
            else
                out_ << "  " << r << " = " << iop << " i32 " << lv << ", " << rv << "\n";
        }
        return r;
    }
 
    if (auto* n = dynamic_cast<const UnopExpr*>(&e)) {
        std::string v = emit_expr(*n->operand);
        std::string r = new_tmp();
        if (n->op == "!") {
            out_ << "  " << r << " = xor i1 " << v << ", 1\n";
        } else { // "-"
            if (n->operand->type.prim == PrimType::Float)
                out_ << "  " << r << " = fneg double " << v << "\n";
            else
                out_ << "  " << r << " = sub i32 0, " << v << "\n";
        }
        return r;
    }
 
    if (auto* n = dynamic_cast<const CallExpr*>(&e)) {
        // collect args
        std::vector<std::pair<std::string,std::string>> evaled;
        for (auto& a : n->args) {
            std::string av = emit_expr(*a);
            evaled.push_back({llvm_type(a->type), av});
        }
        std::string callee_v = emit_expr(*n->callee);
        std::string r = new_tmp();
        std::string ret_t = llvm_type(e.type);
        out_ << "  " << r << " = call " << ret_t << " " << callee_v << "(";
        for (std::size_t i = 0; i < evaled.size(); ++i) {
            if (i) out_ << ", ";
            out_ << evaled[i].first << " " << evaled[i].second;
        }
        out_ << ")\n";
        return r;
    }
 
    if (auto* n = dynamic_cast<const IfExpr*>(&e)) {
        std::string cond   = emit_expr(*n->cond);
        std::string l_then = new_label("ife_then");
        std::string l_else = new_label("ife_else");
        std::string l_end  = new_label("ife_end");
        std::string ty     = llvm_type(e.type);
        std::string phi_r  = new_tmp();
 
        out_ << "  br i1 " << cond
             << ", label %" << l_then
             << ", label %" << l_else << "\n";
        out_ << l_then << ":\n";
        std::string tv = emit_expr(*n->then_expr);
        out_ << "  br label %" << l_end << "\n";
        out_ << l_else << ":\n";
        std::string ev = emit_expr(*n->else_expr);
        out_ << "  br label %" << l_end << "\n";
        out_ << l_end << ":\n";
        out_ << "  " << phi_r << " = phi " << ty
             << " [ " << tv << ", %" << l_then << " ]"
             << ", [ " << ev << ", %" << l_else << " ]\n";
        return phi_r;
    }
 
    throw std::runtime_error("Unknown expr in codegen");
}
 