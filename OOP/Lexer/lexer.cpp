#pragma once

#include "lexer.h"
#include <stdexcept>

inline Lexer::Lexer(std::string text) : text_(text), position_(0) {}

inline TokenType Lexer::char_to_token(char c) {
    switch (c) {
        case '+':
            return TokenType::PLUS;
        case '-':
            return TokenType::MINUS;
        case '*':
            return TokenType::MULTIPLY;
        case '/':
            return TokenType::DIVIDE;
        case '(':
            return TokenType::LPAREN;
        case ')':
            return TokenType::RPAREN;
        default:
            throw std::runtime_error(std::string("Unknown symbol :") + c);
    }
}

inline std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    LexerState state = LexerState::START;
    std::string buffer;

    while (true) {
        char c = (position_ < static_cast<std::size_t>(text_.size())) ? text_[position_] : '\0';

        if (state == LexerState::START) {
            if (c == '\0') {
                break;
            }

            if (std::isspace(c)) {
                ++position_;
                continue;
            }

            if (std::isdigit(c)) {
                buffer += c;
                state = LexerState::IN_NUMBER;
                ++position_;
            } else if (std::isalpha(c) || c == '_') {
                buffer += c;
                state = LexerState::IN_VARIABLE;
                ++position_;
            } else {
                TokenType type = char_to_token(c);
                tokens.push_back({type, std::string(1, c)});
                ++position_;
            }
        } else if (state == LexerState::IN_NUMBER) {
            if (c != '\0' && (std::isdigit(c) || c == '.')) {
                buffer += c;
                ++position_;
            } else {
                tokens.push_back({TokenType::NUMBER, buffer});
                buffer.clear();
                state = LexerState::START;
            }
        } else {
            if (c != '\0' && (std::isalnum(c) || c == '_')) {
                buffer += c;
                ++position_;
            } else {
                tokens.push_back({TokenType::VARIABLE, buffer });
                buffer.clear();
                state = LexerState::START;
            }
        }
    }

    tokens.push_back({TokenType::END, ""});
    return tokens;
}