#include "lexer.h"
#include <stdexcept>
#include <unordered_map>
 
static const std::unordered_map<std::string, TokenType> KEYWORDS = {
    {"fn",     TokenType::KW_FN},
    {"let",    TokenType::KW_LET},
    {"return", TokenType::KW_RETURN},
    {"if",     TokenType::KW_IF},
    {"else",   TokenType::KW_ELSE},
    {"true",   TokenType::KW_TRUE},
    {"false",  TokenType::KW_FALSE},
    {"int",    TokenType::TY_INT},
    {"float",  TokenType::TY_FLOAT},
    {"bool",   TokenType::TY_BOOL},
};
 
Lexer::Lexer(std::string source) : src_(std::move(source)) {}
 
char Lexer::peek(int offset) const {
    std::size_t i = pos_ + offset;
    return (i < src_.size()) ? src_[i] : '\0';
}
 
char Lexer::advance() {
    char c = src_[pos_++];
    if (c == '\n') { ++line_; col_ = 1; }
    else           { ++col_; }
    return c;
}
 
void Lexer::skip_whitespace_and_comments() {
    while (pos_ < src_.size()) {
        char c = peek();
        if (std::isspace(c)) {
            advance();
        } else if (c == '/' && peek(1) == '/') {
            // line comment
            while (pos_ < src_.size() && peek() != '\n')
                advance();
        } else {
            break;
        }
    }
}
 
Token Lexer::make(TokenType t, std::string val) const {
    return {t, std::move(val), line_, col_};
}
 
Token Lexer::read_number() {
    std::string buf;
    bool is_float = false;
    while (pos_ < src_.size() && (std::isdigit(peek()) || peek() == '.')) {
        if (peek() == '.') {
            if (is_float)
                throw std::runtime_error("Malformed float at line " + std::to_string(line_));
            is_float = true;
        }
        buf += advance();
    }
    return make(is_float ? TokenType::FLOAT_LIT : TokenType::INT_LIT, buf);
}
 
Token Lexer::read_ident_or_keyword() {
    std::string buf;
    while (pos_ < src_.size() && (std::isalnum(peek()) || peek() == '_'))
        buf += advance();
 
    auto it = KEYWORDS.find(buf);
    if (it != KEYWORDS.end())
        return make(it->second, buf);
    return make(TokenType::IDENT, buf);
}
 
std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
 
    while (true) {
        skip_whitespace_and_comments();
        if (pos_ >= src_.size()) {
            tokens.push_back(make(TokenType::END_OF_FILE, ""));
            break;
        }
 
        char c = peek();
 
        if (std::isdigit(c)) {
            tokens.push_back(read_number());
            continue;
        }
        if (std::isalpha(c) || c == '_') {
            tokens.push_back(read_ident_or_keyword());
            continue;
        }
 
        // Single / double char operators
        advance(); // consume c
        switch (c) {
            case '+': tokens.push_back(make(TokenType::PLUS,      "+")); break;
            case '*': tokens.push_back(make(TokenType::STAR,      "*")); break;
            case '/': tokens.push_back(make(TokenType::SLASH,     "/")); break;
            case ':': tokens.push_back(make(TokenType::COLON,     ":")); break;
            case ',': tokens.push_back(make(TokenType::COMMA,     ",")); break;
            case ';': tokens.push_back(make(TokenType::SEMICOLON, ";")); break;
            case '(': tokens.push_back(make(TokenType::LPAREN,    "(")); break;
            case ')': tokens.push_back(make(TokenType::RPAREN,    ")")); break;
            case '{': tokens.push_back(make(TokenType::LBRACE,    "{")); break;
            case '}': tokens.push_back(make(TokenType::RBRACE,    "}")); break;
 
            case '-':
                if (peek() == '>') { advance(); tokens.push_back(make(TokenType::ARROW, "->")); }
                else               { tokens.push_back(make(TokenType::MINUS, "-")); }
                break;
            case '=':
                if (peek() == '=') { advance(); tokens.push_back(make(TokenType::EQ_EQ,    "==")); }
                else               { tokens.push_back(make(TokenType::EQ,                  "=")); }
                break;
            case '!':
                if (peek() == '=') { advance(); tokens.push_back(make(TokenType::BANG_EQ,  "!=")); }
                else               { tokens.push_back(make(TokenType::BANG,                "!")); }
                break;
            case '<':
                if (peek() == '=') { advance(); tokens.push_back(make(TokenType::LT_EQ,    "<=")); }
                else               { tokens.push_back(make(TokenType::LT,                  "<")); }
                break;
            case '>':
                if (peek() == '=') { advance(); tokens.push_back(make(TokenType::GT_EQ,    ">=")); }
                else               { tokens.push_back(make(TokenType::GT,                  ">")); }
                break;
            case '&':
                if (peek() == '&') { advance(); tokens.push_back(make(TokenType::AND_AND,  "&&")); }
                else throw std::runtime_error("Unexpected '&' at line " + std::to_string(line_));
                break;
            case '|':
                if (peek() == '|') { advance(); tokens.push_back(make(TokenType::OR_OR,    "||")); }
                else throw std::runtime_error("Unexpected '|' at line " + std::to_string(line_));
                break;
 
            default:
                throw std::runtime_error(
                    std::string("Unknown character '") + c +
                    "' at line " + std::to_string(line_));
        }
    }
    return tokens;
}