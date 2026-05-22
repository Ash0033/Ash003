#include "interpreter.h"
#include <cmath>
#include <stdexcept>
 
// ── Helpers ──────────────────────────────────────────────────────────────────
 
const FnDecl& Interpreter::find_fn(const std::string& name, int line) const {
    for (auto& fn : prog_.functions)
        if (fn.name == name) return fn;
    throw std::runtime_error("Line " + std::to_string(line) +
                             ": undefined function '" + name + "'");
}
 
void Interpreter::notify(int line, const std::string& desc) {
    state_.log_event(line, desc);
    if (state_.should_pause(line)) {
        state_.paused = true;
        on_pause_(state_, line, desc);
        state_.paused = false;
    }
}
 
// ── Entry ────────────────────────────────────────────────────────────────────
 
AshValue Interpreter::run(const std::string& fn_name,
                          std::vector<AshValue> args,
                          int call_line) {
    const FnDecl& fn = find_fn(fn_name, call_line);
 
    if (args.size() != fn.params.size())
        throw std::runtime_error("Wrong number of arguments calling '" + fn_name + "'");
 
    state_.push_frame(fn_name, call_line);
    notify(fn.line, "→ call " + fn_name);
 
    for (std::size_t i = 0; i < fn.params.size(); ++i)
        state_.set_local(fn.params[i].name, args[i]);
 
    AshValue result = AshValue::make_int(0);
    try {
        for (auto& s : fn.body) exec_stmt(*s);
    } catch (ReturnSignal& ret) {
        result = ret.value;
    }
 
    notify(fn.line, "← return from " + fn_name + " = " + result.to_string());
    state_.pop_frame();
    return result;
}
 
// ── Statements ───────────────────────────────────────────────────────────────
 
void Interpreter::exec_stmt(const Stmt& s) {
    if (auto* let = dynamic_cast<const LetStmt*>(&s)) {
        AshValue val = eval_expr(*let->init);
        notify(let->line, "let " + let->name + " = " + val.to_string());
        state_.set_local(let->name, val);
        return;
    }
 
    if (auto* ret = dynamic_cast<const ReturnStmt*>(&s)) {
        AshValue val = eval_expr(*ret->value);
        notify(ret->line, "return " + val.to_string());
        throw ReturnSignal{val};
    }
 
    if (auto* ifs = dynamic_cast<const IfStmt*>(&s)) {
        AshValue cond = eval_expr(*ifs->cond);
        notify(ifs->line, "if (" + cond.to_string() + ")");
        if (cond.as_bool) {
            for (auto& st : ifs->then_body) exec_stmt(*st);
        } else {
            for (auto& st : ifs->else_body) exec_stmt(*st);
        }
        return;
    }
 
    if (auto* es = dynamic_cast<const ExprStmt*>(&s)) {
        AshValue val = eval_expr(*es->expr);
        notify(es->line, "expr → " + val.to_string());
        return;
    }
 
    throw std::runtime_error("Unknown statement in interpreter");
}
 
// ── Expressions ──────────────────────────────────────────────────────────────
 
AshValue Interpreter::eval_expr(const Expr& e) {
    if (auto* n = dynamic_cast<const IntLitExpr*>(&e))
        return AshValue::make_int(n->value);
 
    if (auto* n = dynamic_cast<const FloatLitExpr*>(&e))
        return AshValue::make_float(n->value);
 
    if (auto* n = dynamic_cast<const BoolLitExpr*>(&e))
        return AshValue::make_bool(n->value);
 
    if (auto* n = dynamic_cast<const IdentExpr*>(&e)) {
        // Could be a local variable or a function name
        auto v = state_.get_local(n->name);
        if (v) return *v;
        // Check if it's a function name
        for (auto& fn : prog_.functions)
            if (fn.name == n->name) return AshValue::make_fn(n->name);
        throw std::runtime_error("Line " + std::to_string(n->line) +
                                 ": undefined '" + n->name + "'");
    }
 
    if (auto* n = dynamic_cast<const BinopExpr*>(&e)) {
        AshValue l = eval_expr(*n->lhs);
        // Short-circuit for && and ||
        if (n->op == "&&") {
            if (!l.as_bool) return AshValue::make_bool(false);
            return AshValue::make_bool(eval_expr(*n->rhs).as_bool);
        }
        if (n->op == "||") {
            if (l.as_bool) return AshValue::make_bool(true);
            return AshValue::make_bool(eval_expr(*n->rhs).as_bool);
        }
        AshValue r = eval_expr(*n->rhs);
        return apply_binop(n->op, l, r, n->line);
    }
 
    if (auto* n = dynamic_cast<const UnopExpr*>(&e)) {
        AshValue v = eval_expr(*n->operand);
        if (n->op == "!") return AshValue::make_bool(!v.as_bool);
        if (n->op == "-") {
            if (v.kind == AshValue::Kind::Float) return AshValue::make_float(-v.as_float);
            return AshValue::make_int(-v.as_int);
        }
    }
 
    if (auto* n = dynamic_cast<const CallExpr*>(&e)) {
        AshValue callee = eval_expr(*n->callee);
        if (callee.kind != AshValue::Kind::Fn)
            throw std::runtime_error("Line " + std::to_string(n->line) + ": not callable");
 
        std::vector<AshValue> args;
        for (auto& a : n->args) args.push_back(eval_expr(*a));
 
        return run(callee.as_fn, std::move(args), n->line);
    }
 
    if (auto* n = dynamic_cast<const IfExpr*>(&e)) {
        AshValue cond = eval_expr(*n->cond);
        return cond.as_bool ? eval_expr(*n->then_expr) : eval_expr(*n->else_expr);
    }
 
    throw std::runtime_error("Unknown expression in interpreter");
}
 
// ── Binary operator ───────────────────────────────────────────────────────────
 
AshValue Interpreter::apply_binop(const std::string& op,
                                  const AshValue& l,
                                  const AshValue& r,
                                  int line) {
    bool is_float = (l.kind == AshValue::Kind::Float);
 
    // Arithmetic
    if (op == "+") {
        return is_float
            ? AshValue::make_float(l.as_float + r.as_float)
            : AshValue::make_int  (l.as_int   + r.as_int);
    }
    if (op == "-") {
        return is_float
            ? AshValue::make_float(l.as_float - r.as_float)
            : AshValue::make_int  (l.as_int   - r.as_int);
    }
    if (op == "*") {
        return is_float
            ? AshValue::make_float(l.as_float * r.as_float)
            : AshValue::make_int  (l.as_int   * r.as_int);
    }
    if (op == "/") {
        if (is_float) return AshValue::make_float(l.as_float / r.as_float);
        if (r.as_int == 0)
            throw std::runtime_error("Line " + std::to_string(line) + ": division by zero");
        return AshValue::make_int(l.as_int / r.as_int);
    }
 
    // Comparison
    auto cmp_float = [&](auto fn) { return AshValue::make_bool(fn(l.as_float, r.as_float)); };
    auto cmp_int   = [&](auto fn) { return AshValue::make_bool(fn(l.as_int,   r.as_int));   };
    auto cmp_bool  = [&](auto fn) { return AshValue::make_bool(fn(l.as_bool,  r.as_bool));  };
 
    auto cmp = [&](auto fn) {
        if (l.kind == AshValue::Kind::Float) return cmp_float(fn);
        if (l.kind == AshValue::Kind::Bool)  return cmp_bool(fn);
        return cmp_int(fn);
    };
 
    if (op == "==") return cmp([](auto a, auto b){ return a == b; });
    if (op == "!=") return cmp([](auto a, auto b){ return a != b; });
    if (op == "<")  return cmp([](auto a, auto b){ return a <  b; });
    if (op == ">")  return cmp([](auto a, auto b){ return a >  b; });
    if (op == "<=") return cmp([](auto a, auto b){ return a <= b; });
    if (op == ">=") return cmp([](auto a, auto b){ return a >= b; });
 
    throw std::runtime_error("Line " + std::to_string(line) +
                             ": unknown operator '" + op + "'");
}