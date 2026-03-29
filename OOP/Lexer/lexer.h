#pragma once

#include "lexer.h"
#include <string>
#include <vector>

enum class LexerState {
    START,
    IN_NUMBER,
    IN_VARIABLE
};

class Lexer {
private:
    std::string text_;
    std::size_t position_;

    TokenType char_to_token(char c);
public:
    explicit Lexer(std::string text);

    std::vector<Token> tokenize();
};

#include "lexer.cpp"