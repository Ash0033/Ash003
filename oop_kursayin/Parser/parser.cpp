#include "parser.h"
#include <stdexcept>
 
// ── Helpers ──────────────────────────────────────────────────────────────────
 
Parser::Parser(std::vector<Token> tokens) : tokens_(std::move(tokens)) {}
 
Token& Parser::cur()                { return tokens_[pos_]; }
Token& Parser::peek(int offset)     { return tokens_[pos_ + offset]; }
Token  Parser::eat()                { return tokens_[pos_++]; }
bool   Parser::check(TokenType t) const { return tokens_[pos_].type == t; }
 
bool Parser::match(TokenType t) {
    if (check(t)) { ++pos_; return true; }
    return false;
}
 
Token Parser::expect(TokenType t) {
    if (!check(t)) {
        throw std::runtime_error(
            "Line " + std::to_string(cur().line) +
            ": expected token, got '" + cur().value + "'");
    }
    return eat();
}
 
// ── Type parser ──────────────────────────────────────────────────────────────
 
AshType Parser::parse_type() {
    AshType t;
    if (check(TokenType::TY_INT))   { eat(); t.prim = PrimType::Int;   return t; }
    if (check(TokenType::TY_FLOAT)) { eat(); t.prim = PrimType::Float; return t; }
    if (check(TokenType::TY_BOOL))  { eat(); t.prim = PrimType::Bool;  return t; }
 
    // fn(int, float) -> bool
    if (check(TokenType::KW_FN)) {
        eat();
        t.is_fn = true;
        expect(TokenType::LPAREN);
        while (!check(TokenType::RPAREN)) {
            t.param_types.push_back(parse_type());
            if (!match(TokenType::COMMA)) break;
        }
        expect(TokenType::RPAREN);
        expect(TokenType::ARROW);
        t.ret_type = std::make_unique<AshType>(parse_type());
        return t;
    }
 
    throw std::runtime_error("Line " + std::to_string(cur().line) + ": expected type");
}
 
// ── Program / Functions ──────────────────────────────────────────────────────
 
Program Parser::parse() {
    Program prog;
    while (!check(TokenType::END_OF_FILE))
        prog.functions.push_back(parse_fn());
    return prog;
}
 
Param Parser::parse_param() {
    Param p;
    p.name = expect(TokenType::IDENT).value;
    expect(TokenType::COLON);
    p.type = parse_type();
    return p;
}
 
FnDecl Parser::parse_fn() {
    FnDecl fn;
    fn.line = cur().line;
    expect(TokenType::KW_FN);
    fn.name = expect(TokenType::IDENT).value;
    expect(TokenType::LPAREN);
    while (!check(TokenType::RPAREN)) {
        fn.params.push_back(parse_param());
        if (!match(TokenType::COMMA)) break;
    }
    expect(TokenType::RPAREN);
    expect(TokenType::ARROW);
    fn.ret_type = parse_type();
    fn.body = parse_block();
    return fn;
}
 
std::vector<StmtPtr> Parser::parse_block() {
    expect(TokenType::LBRACE);
    std::vector<StmtPtr> stmts;
    while (!check(TokenType::RBRACE))
        stmts.push_back(parse_stmt());
    expect(TokenType::RBRACE);
    return stmts;
}
 
// ── Statements ───────────────────────────────────────────────────────────────
 
StmtPtr Parser::parse_stmt() {
    if (check(TokenType::KW_LET))    return parse_let();
    if (check(TokenType::KW_RETURN)) return parse_return();
    if (check(TokenType::KW_IF))     return parse_if_stmt();
 
    // expression statement
    int ln = cur().line;
    auto e = parse_expr();
    expect(TokenType::SEMICOLON);
    return std::make_unique<ExprStmt>(std::move(e), ln);
}
 
StmtPtr Parser::parse_let() {
    int ln = cur().line;
    expect(TokenType::KW_LET);
    std::string name = expect(TokenType::IDENT).value;
    expect(TokenType::COLON);
    AshType ty = parse_type();
    expect(TokenType::EQ);
    auto init = parse_expr();
    expect(TokenType::SEMICOLON);
    return std::make_unique<LetStmt>(std::move(name), std::move(ty), std::move(init), ln);
}
 
StmtPtr Parser::parse_return() {
    int ln = cur().line;
    expect(TokenType::KW_RETURN);
    auto val = parse_expr();
    expect(TokenType::SEMICOLON);
    return std::make_unique<ReturnStmt>(std::move(val), ln);
}
 
StmtPtr Parser::parse_if_stmt() {
    int ln = cur().line;
    expect(TokenType::KW_IF);
    expect(TokenType::LPAREN);
    auto cond = parse_expr();
    expect(TokenType::RPAREN);
    auto then_body = parse_block();
    std::vector<StmtPtr> else_body;
    if (match(TokenType::KW_ELSE))
        else_body = parse_block();
    return std::make_unique<IfStmt>(std::move(cond), std::move(then_body), std::move(else_body), ln);
}
 
// ── Expressions (Pratt-style precedence climbing) ────────────────────────────
 
ExprPtr Parser::parse_expr()           { return parse_or(); }
 
ExprPtr Parser::parse_or() {
    auto lhs = parse_and();
    while (check(TokenType::OR_OR)) {
        int ln = cur().line; eat();
        lhs = std::make_unique<BinopExpr>("||", std::move(lhs), parse_and(), ln);
    }
    return lhs;
}
 
ExprPtr Parser::parse_and() {
    auto lhs = parse_equality();
    while (check(TokenType::AND_AND)) {
        int ln = cur().line; eat();
        lhs = std::make_unique<BinopExpr>("&&", std::move(lhs), parse_equality(), ln);
    }
    return lhs;
}
 
ExprPtr Parser::parse_equality() {
    auto lhs = parse_comparison();
    while (check(TokenType::EQ_EQ) || check(TokenType::BANG_EQ)) {
        int ln = cur().line;
        std::string op = eat().value;
        lhs = std::make_unique<BinopExpr>(op, std::move(lhs), parse_comparison(), ln);
    }
    return lhs;
}
 
ExprPtr Parser::parse_comparison() {
    auto lhs = parse_additive();
    while (check(TokenType::LT) || check(TokenType::GT) ||
           check(TokenType::LT_EQ) || check(TokenType::GT_EQ)) {
        int ln = cur().line;
        std::string op = eat().value;
        lhs = std::make_unique<BinopExpr>(op, std::move(lhs), parse_additive(), ln);
    }
    return lhs;
}
 
ExprPtr Parser::parse_additive() {
    auto lhs = parse_multiplicative();
    while (check(TokenType::PLUS) || check(TokenType::MINUS)) {
        int ln = cur().line;
        std::string op = eat().value;
        lhs = std::make_unique<BinopExpr>(op, std::move(lhs), parse_multiplicative(), ln);
    }
    return lhs;
}
 
ExprPtr Parser::parse_multiplicative() {
    auto lhs = parse_unary();
    while (check(TokenType::STAR) || check(TokenType::SLASH)) {
        int ln = cur().line;
        std::string op = eat().value;
        lhs = std::make_unique<BinopExpr>(op, std::move(lhs), parse_unary(), ln);
    }
    return lhs;
}
 
ExprPtr Parser::parse_unary() {
    if (check(TokenType::BANG) || check(TokenType::MINUS)) {
        int ln = cur().line;
        std::string op = eat().value;
        return std::make_unique<UnopExpr>(op, parse_unary(), ln);
    }
    return parse_call();
}
 
ExprPtr Parser::parse_call() {
    auto callee = parse_primary();
    while (check(TokenType::LPAREN)) {
        int ln = cur().line;
        eat(); // (
        std::vector<ExprPtr> args;
        while (!check(TokenType::RPAREN)) {
            args.push_back(parse_expr());
            if (!match(TokenType::COMMA)) break;
        }
        expect(TokenType::RPAREN);
        callee = std::make_unique<CallExpr>(std::move(callee), std::move(args), ln);
    }
    return callee;
}
 
ExprPtr Parser::parse_primary() {
    int ln = cur().line;
 
    if (check(TokenType::INT_LIT)) {
        int64_t v = std::stoll(eat().value);
        return std::make_unique<IntLitExpr>(v, ln);
    }
    if (check(TokenType::FLOAT_LIT)) {
        double v = std::stod(eat().value);
        return std::make_unique<FloatLitExpr>(v, ln);
    }
    if (check(TokenType::KW_TRUE))  { eat(); return std::make_unique<BoolLitExpr>(true,  ln); }
    if (check(TokenType::KW_FALSE)) { eat(); return std::make_unique<BoolLitExpr>(false, ln); }
 
    if (check(TokenType::IDENT)) {
        std::string name = eat().value;
        return std::make_unique<IdentExpr>(std::move(name), ln);
    }
 
    if (match(TokenType::LPAREN)) {
        auto e = parse_expr();
        expect(TokenType::RPAREN);
        return e;
    }
 
    // if as expression: if(cond) { expr } else { expr }
    if (check(TokenType::KW_IF)) {
        eat();
        expect(TokenType::LPAREN);
        auto cond = parse_expr();
        expect(TokenType::RPAREN);
        expect(TokenType::LBRACE);
        auto then_e = parse_expr();
        expect(TokenType::RBRACE);
        expect(TokenType::KW_ELSE);
        expect(TokenType::LBRACE);
        auto else_e = parse_expr();
        expect(TokenType::RBRACE);
        return std::make_unique<IfExpr>(std::move(cond), std::move(then_e), std::move(else_e), ln);
    }
 
    throw std::runtime_error(
        "Line " + std::to_string(ln) + ": unexpected token '" + cur().value + "'");
}
 