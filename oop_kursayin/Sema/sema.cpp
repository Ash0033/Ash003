#include "sema.h"
#include <stdexcept>
 
// ── Helpers ──────────────────────────────────────────────────────────────────
 
std::string Sema::type_str(const AshType& t) {
    if (t.is_fn) {
        std::string s = "fn(";
        for (std::size_t i = 0; i < t.param_types.size(); ++i) {
            if (i) s += ", ";
            s += type_str(t.param_types[i]);
        }
        s += ") -> ";
        s += t.ret_type ? type_str(*t.ret_type) : "void";
        return s;
    }
    switch (t.prim) {
        case PrimType::Int:   return "int";
        case PrimType::Float: return "float";
        case PrimType::Bool:  return "bool";
        case PrimType::Void:  return "void";
    }
    return "?";
}
 
void Sema::push_scope() { scopes_.emplace_back(); }
void Sema::pop_scope()  { scopes_.pop_back(); }
 
void Sema::define(const std::string& name, AshType t, int line) {
    auto& top = scopes_.back();
    if (top.count(name))
        throw std::runtime_error("Line " + std::to_string(line) +
            ": '" + name + "' already defined in this scope");
    top[name] = std::move(t);
}
 
const AshType& Sema::lookup(const std::string& name, int line) const {
    for (int i = (int)scopes_.size() - 1; i >= 0; --i) {
        auto it = scopes_[i].find(name);
        if (it != scopes_[i].end()) return it->second;
    }
    throw std::runtime_error("Line " + std::to_string(line) +
        ": undefined name '" + name + "'");
}
 
// ── Entry ────────────────────────────────────────────────────────────────────
 
void Sema::check(Program& prog) {
    // First pass: register all function signatures so mutual calls work
    push_scope();
    for (auto& fn : prog.functions) {
        AshType fn_type;
        fn_type.is_fn = true;
        for (auto& p : fn.params) fn_type.param_types.push_back(p.type.clone());
        fn_type.ret_type = std::make_unique<AshType>(fn.ret_type.clone());
        define(fn.name, std::move(fn_type), fn.line);
    }
 
    // Second pass: type-check bodies
    for (auto& fn : prog.functions) check_fn(fn);
    pop_scope();
}
 
// ── Function ─────────────────────────────────────────────────────────────────
 
void Sema::check_fn(FnDecl& fn) {
    current_ret_type_ = fn.ret_type.clone();
    push_scope();
    for (auto& p : fn.params) define(p.name, p.type.clone(), fn.line);
    for (auto& s : fn.body)   check_stmt(*s);
    pop_scope();
}
 
// ── Statements ───────────────────────────────────────────────────────────────
 
void Sema::check_stmt(Stmt& s) {
    if (auto* let = dynamic_cast<LetStmt*>(&s)) {
        AshType init_t = check_expr(*let->init);
        if (init_t != let->declared_type)
            throw std::runtime_error(
                "Line " + std::to_string(let->line) +
                ": type mismatch in let: declared '" + type_str(let->declared_type) +
                "', got '" + type_str(init_t) + "'");
        define(let->name, let->declared_type.clone(), let->line);
        return;
    }
    if (auto* ret = dynamic_cast<ReturnStmt*>(&s)) {
        AshType vt = check_expr(*ret->value);
        if (vt != current_ret_type_)
            throw std::runtime_error(
                "Line " + std::to_string(ret->line) +
                ": return type mismatch: expected '" + type_str(current_ret_type_) +
                "', got '" + type_str(vt) + "'");
        return;
    }
    if (auto* ifs = dynamic_cast<IfStmt*>(&s)) {
        AshType ct = check_expr(*ifs->cond);
        if (ct.prim != PrimType::Bool || ct.is_fn)
            throw std::runtime_error(
                "Line " + std::to_string(ifs->line) + ": if condition must be bool");
        push_scope();
        for (auto& st : ifs->then_body) check_stmt(*st);
        pop_scope();
        if (!ifs->else_body.empty()) {
            push_scope();
            for (auto& st : ifs->else_body) check_stmt(*st);
            pop_scope();
        }
        return;
    }
    if (auto* es = dynamic_cast<ExprStmt*>(&s)) {
        check_expr(*es->expr);
        return;
    }
    throw std::runtime_error("Unknown statement node in sema");
}
 
// ── Expressions ──────────────────────────────────────────────────────────────
 
AshType Sema::check_expr(Expr& e) {
    if (auto* n = dynamic_cast<IntLitExpr*>(&e)) {
        e.type = AshType(PrimType::Int); return e.type;
    }
    if (auto* n = dynamic_cast<FloatLitExpr*>(&e)) {
        e.type = AshType(PrimType::Float); return e.type;
    }
    if (auto* n = dynamic_cast<BoolLitExpr*>(&e)) {
        e.type = AshType(PrimType::Bool); return e.type;
    }
    if (auto* n = dynamic_cast<IdentExpr*>(&e)) {
        e.type = lookup(n->name, n->line).clone(); return e.type;
    }
 
    if (auto* n = dynamic_cast<BinopExpr*>(&e)) {
        AshType lt = check_expr(*n->lhs);
        AshType rt = check_expr(*n->rhs);
 
        // Boolean ops
        if (n->op == "&&" || n->op == "||") {
            if (lt.prim != PrimType::Bool || rt.prim != PrimType::Bool || lt.is_fn || rt.is_fn)
                throw std::runtime_error("Line " + std::to_string(n->line) +
                    ": '" + n->op + "' requires bool operands");
            e.type = AshType(PrimType::Bool); return e.type;
        }
        // Comparison
        if (n->op == "==" || n->op == "!=" ||
            n->op == "<"  || n->op == ">"  ||
            n->op == "<=" || n->op == ">=") {
            if (lt != rt)
                throw std::runtime_error("Line " + std::to_string(n->line) +
                    ": comparison between different types");
            e.type = AshType(PrimType::Bool); return e.type;
        }
        // Arithmetic
        if (lt != rt || lt.is_fn)
            throw std::runtime_error("Line " + std::to_string(n->line) +
                ": arithmetic requires matching numeric types, got '" +
                type_str(lt) + "' and '" + type_str(rt) + "'");
        if (lt.prim == PrimType::Bool)
            throw std::runtime_error("Line " + std::to_string(n->line) +
                ": arithmetic not allowed on bool");
        e.type = lt; return e.type;
    }
 
    if (auto* n = dynamic_cast<UnopExpr*>(&e)) {
        AshType ot = check_expr(*n->operand);
        if (n->op == "!") {
            if (ot.prim != PrimType::Bool || ot.is_fn)
                throw std::runtime_error("Line " + std::to_string(n->line) + ": '!' requires bool");
            e.type = AshType(PrimType::Bool); return e.type;
        }
        if (n->op == "-") {
            if (ot.prim == PrimType::Bool || ot.is_fn)
                throw std::runtime_error("Line " + std::to_string(n->line) + ": '-' requires numeric");
            e.type = ot; return e.type;
        }
    }
 
    if (auto* n = dynamic_cast<CallExpr*>(&e)) {
        AshType ct = check_expr(*n->callee);
        if (!ct.is_fn)
            throw std::runtime_error("Line " + std::to_string(n->line) + ": not callable");
        if (ct.param_types.size() != n->args.size())
            throw std::runtime_error("Line " + std::to_string(n->line) +
                ": wrong number of arguments");
        for (std::size_t i = 0; i < n->args.size(); ++i) {
            AshType at = check_expr(*n->args[i]);
            if (at != ct.param_types[i])
                throw std::runtime_error("Line " + std::to_string(n->line) +
                    ": argument " + std::to_string(i) + " type mismatch");
        }
        e.type = ct.ret_type->clone(); return e.type;
    }
 
    if (auto* n = dynamic_cast<IfExpr*>(&e)) {
        AshType ct = check_expr(*n->cond);
        if (ct.prim != PrimType::Bool || ct.is_fn)
            throw std::runtime_error("Line " + std::to_string(n->line) + ": if condition must be bool");
        AshType tt = check_expr(*n->then_expr);
        AshType et = check_expr(*n->else_expr);
        if (tt != et)
            throw std::runtime_error("Line " + std::to_string(n->line) +
                ": if branches have different types");
        e.type = tt; return e.type;
    }
 
    throw std::runtime_error("Unknown expression node in sema");
}
 