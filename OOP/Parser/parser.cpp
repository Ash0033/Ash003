#pragma once

#include "parser.h"
#include <stdexcept>
#include "../helper.h"
#include "../Abstract Syntax Tree/num_node.h"
#include "../Abstract Syntax Tree/v_node.h"
#include "../Abstract Syntax Tree/binop_node.h"

inline Parser::Parser(std::vector<Token> tokens) : tokens_(std::move(tokens)), position_(0) {}

inline Token& Parser::current() {
    return tokens_[position_];
}

inline Token Parser::eat() {
    return tokens_[position_++];
}

inline void Parser::expect(TokenType type) {
    if (current().type != type) {
        throw std::runtime_error("It was expected '" + token_type_name(type) +
            "' received '" + token_type_name(current().type) + "'");
    }
    ++position_;
}

inline std::unique_ptr<BaseNode> Parser::parse_expression() {
    std::unique_ptr<BaseNode> node = parse_term();
    while (current().type == TokenType::PLUS || current().type == TokenType::MINUS) {
        char operator_binary = current().value[0];
        ++position_;
        node = std::make_unique<BinaryOperatorNode>(operator_binary, std::move(node),parse_term());
    }
    return node;
}

inline std::unique_ptr<BaseNode> Parser::parse_term() {
    std::unique_ptr<BaseNode> node = parse_factor();
    while (current().type == TokenType::MULTIPLY || current().type == TokenType::DIVIDE) {
        char operator_binary = current().value[0];
        ++position_;
        node = std::make_unique<BinaryOperatorNode>(operator_binary, std::move(node), parse_factor());
    }
    return node;
}

inline std::unique_ptr<BaseNode> Parser::parse_factor() {
    Token token = current();

    if (token.type == TokenType::NUMBER) {
        ++position_;
        return std::make_unique<NumberNode>(std::stod(token.value));
    }
    if (token.type == TokenType::VARIABLE) {
        ++position_;
        return std::make_unique<VariableNode>(token.value);
    }
    if (token.type == TokenType::LPAREN) {
        expect(TokenType::LPAREN);
        std::unique_ptr<BaseNode> node = parse_expression();
        expect(TokenType::RPAREN);
        return node;
    }

    throw std::runtime_error(std::string("Unknown token '") + token.value + "'");
}

inline std::unique_ptr<BaseNode> Parser::parse()  {
    std::unique_ptr<BaseNode> tree = parse_expression();
    expect(TokenType::END);
    return tree;
}