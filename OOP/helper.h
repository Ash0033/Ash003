#pragma once

#include "./Type/op_code.h"

inline std::string token_type_name(TokenType type) {
    switch (type) {
        case TokenType::NUMBER:
            return "NUMBER";
        case TokenType::VARIABLE:
            return "VARIABLE";
        case TokenType::PLUS:
            return "PLUS";
        case TokenType::MINUS:
            return "MINUS";
        case TokenType::MULTIPLY:
            return "MULTIPLY";
        case TokenType::DIVIDE:
            return "DIVIDE";
        case TokenType::LPAREN:
            return "LPAREN";
        case TokenType::RPAREN:
            return "RPAREN";
        case TokenType::END:
            return "END";
        default:
            return "?";
    }
}

inline std::string operator_code_name(OperationCode operator_code) {
    switch (operator_code) {
        case OperationCode::MOVE:
            return "MOVE";
        case OperationCode::LOAD:
            return "LOAD";
        case OperationCode::ADD:
            return "ADD";
        case OperationCode::SUB:
            return "SUB";
        case OperationCode::MUL:
            return "MUL";
        case OperationCode::DIV:
            return "DIV";
        default:
            return "?";
    }
}

inline void print_separator(char c = '=', std::size_t size = 55) {
    std::cout << std::string(size, c) << '\n';
}

inline void print_tokens(const std::vector<Token>& tokens) {
    std::cout << "  " << std::left << std::setw(14) << "type:"
              << "value\n";
    std::cout << "  " << std::string(14, ' ') << ":" << std::string(10, '-') << '\n';
    for (const Token& token : tokens) {
        if (token.type == TokenType::END) continue;
        std::cout << "  " << std::setw(14) << token_type_name(token.type)
                  << ": '" << token.value << "'\n";
    }
}