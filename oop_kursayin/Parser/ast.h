#pragma once
#include <memory>
#include <string>
#include <vector>
 
// ── Type representation ──────────────────────────────────────────────────────
enum class PrimType { Int, Float, Bool, Void };
 
struct AshType {
    PrimType prim = PrimType::Int;
    bool     is_fn = false;
    std::vector<AshType> param_types; // for fn types
    std::unique_ptr<AshType> ret_type;
 
    AshType() = default;
    explicit AshType(PrimType p) : prim(p) {}
 
    // Deep copy
    AshType(const AshType& o) {
        prim  = o.prim;
        is_fn = o.is_fn;
        for (auto& pt : o.param_types) param_types.push_back(pt);
        if (o.ret_type) ret_type = std::make_unique<AshType>(*o.ret_type);
    }
    AshType& operator=(const AshType& o) {
        if (this == &o) return *this;
        prim  = o.prim; is_fn = o.is_fn;
        param_types.clear();
        for (auto& pt : o.param_types) param_types.push_back(pt);
        ret_type = o.ret_type ? std::make_unique<AshType>(*o.ret_type) : nullptr;
        return *this;
    }
    AshType(AshType&&) = default;
    AshType& operator=(AshType&&) = default;
 
    AshType clone() const { return *this; }
 
    // Deep copy helper (kept for compatibility)
    AshType clone_deep() const { return *this; }
 
    bool operator==(const AshType& o) const {
        if (prim != o.prim || is_fn != o.is_fn) return false;
        if (is_fn) {
            if (param_types.size() != o.param_types.size()) return false;
            for (std::size_t i = 0; i < param_types.size(); ++i)
                if (!(param_types[i] == o.param_types[i])) return false;
            return *ret_type == *o.ret_type;
        }
        return true;
    }
    bool operator!=(const AshType& o) const { return !(*this == o); }
};
 
// ── Base node ────────────────────────────────────────────────────────────────
struct Expr;
struct Stmt;
using ExprPtr = std::unique_ptr<Expr>;
using StmtPtr = std::unique_ptr<Stmt>;
 
struct Expr {
    AshType type; // filled by semantic analysis
    int line = 0;
    virtual ~Expr() = default;
};
 
struct Stmt {
    int line = 0;
    virtual ~Stmt() = default;
};
 
// ── Expressions ─────────────────────────────────────────────────────────────
 
struct IntLitExpr : Expr {
    int64_t value;
    explicit IntLitExpr(int64_t v, int ln) : value(v) { line = ln; type = AshType(PrimType::Int); }
};
 
struct FloatLitExpr : Expr {
    double value;
    explicit FloatLitExpr(double v, int ln) : value(v) { line = ln; type = AshType(PrimType::Float); }
};
 
struct BoolLitExpr : Expr {
    bool value;
    explicit BoolLitExpr(bool v, int ln) : value(v) { line = ln; type = AshType(PrimType::Bool); }
};
 
struct IdentExpr : Expr {
    std::string name;
    explicit IdentExpr(std::string n, int ln) : name(std::move(n)) { line = ln; }
};
 
struct BinopExpr : Expr {
    std::string op;
    ExprPtr     lhs, rhs;
    BinopExpr(std::string op, ExprPtr l, ExprPtr r, int ln)
        : op(std::move(op)), lhs(std::move(l)), rhs(std::move(r)) { line = ln; }
};
 
struct UnopExpr : Expr {
    std::string op;
    ExprPtr     operand;
    UnopExpr(std::string op, ExprPtr e, int ln)
        : op(std::move(op)), operand(std::move(e)) { line = ln; }
};
 
struct CallExpr : Expr {
    ExprPtr              callee;
    std::vector<ExprPtr> args;
    CallExpr(ExprPtr callee, std::vector<ExprPtr> args, int ln)
        : callee(std::move(callee)), args(std::move(args)) { line = ln; }
};
 
struct IfExpr : Expr {
    ExprPtr cond;
    ExprPtr then_expr;
    ExprPtr else_expr; // required in expression position
    IfExpr(ExprPtr c, ExprPtr t, ExprPtr e, int ln)
        : cond(std::move(c)), then_expr(std::move(t)), else_expr(std::move(e)) { line = ln; }
};
 
// ── Statements ───────────────────────────────────────────────────────────────
 
struct LetStmt : Stmt {
    std::string name;
    AshType     declared_type;
    ExprPtr     init;
    LetStmt(std::string n, AshType t, ExprPtr e, int ln)
        : name(std::move(n)), declared_type(std::move(t)), init(std::move(e)) { line = ln; }
};
 
struct ReturnStmt : Stmt {
    ExprPtr value;
    explicit ReturnStmt(ExprPtr v, int ln) : value(std::move(v)) { line = ln; }
};
 
struct ExprStmt : Stmt {
    ExprPtr expr;
    explicit ExprStmt(ExprPtr e, int ln) : expr(std::move(e)) { line = ln; }
};
 
struct IfStmt : Stmt {
    ExprPtr cond;
    std::vector<StmtPtr> then_body;
    std::vector<StmtPtr> else_body; // may be empty
    IfStmt(ExprPtr c, std::vector<StmtPtr> t, std::vector<StmtPtr> e, int ln)
        : cond(std::move(c)), then_body(std::move(t)), else_body(std::move(e)) { line = ln; }
};
 
// ── Top-level ────────────────────────────────────────────────────────────────
 
struct Param {
    std::string name;
    AshType     type;
};
 
struct FnDecl {
    std::string          name;
    std::vector<Param>   params;
    AshType              ret_type;
    std::vector<StmtPtr> body;
    int                  line = 0;
};
 
struct Program {
    std::vector<FnDecl> functions;
};
 