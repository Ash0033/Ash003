#pragma once
#include "ast.h"
#include "../Lexer/lexer.h"
#include <vector>
 
class Parser {
public:
    explicit Parser(std::vector<Token> tokens);
    Program parse();
 
private:
    std::vector<Token> tokens_;
    std::size_t        pos_ = 0;
 
    Token&       cur();
    Token&       peek(int offset = 1);
    Token        eat();
    Token        expect(TokenType t);
    bool         check(TokenType t) const;
    bool         match(TokenType t);
 
    AshType    parse_type();
    FnDecl     parse_fn();
    Param      parse_param();
 
    StmtPtr    parse_stmt();
    StmtPtr    parse_let();
    StmtPtr    parse_return();
    StmtPtr    parse_if_stmt();
    std::vector<StmtPtr> parse_block();
 
    ExprPtr    parse_expr();
    ExprPtr    parse_or();
    ExprPtr    parse_and();
    ExprPtr    parse_equality();
    ExprPtr    parse_comparison();
    ExprPtr    parse_additive();
    ExprPtr    parse_multiplicative();
    ExprPtr    parse_unary();
    ExprPtr    parse_call();
    ExprPtr    parse_primary();
};
 