#pragma once

#include <vector>
#include <memory>
#include "../Type/token.h"
#include "../Abstract Syntax Tree/ast_node.h"

class Parser {
private:
    std::vector<Token> tokens_;
    std::size_t position_;

    Token& current();
    Token eat();
    void expect(TokenType type);

    std::unique_ptr<BaseNode> parse_expression();
    std::unique_ptr<BaseNode> parse_term();
    std::unique_ptr<BaseNode> parse_factor();
public:
    explicit Parser(std::vector<Token> tokens);

    std::unique_ptr<BaseNode> parse();
};

#include "parser.cpp"