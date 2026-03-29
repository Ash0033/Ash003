#pragma once

#include <string>

enum class TokenType {
    NUMBER,
    VARIABLE,
    PLUS,
    MINUS,
    MULTIPLY,
    DIVIDE,
    LPAREN,
    RPAREN,
    END
};

struct Token {
    TokenType type;
    std::string value;
};