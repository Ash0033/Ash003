#pragma once
#include "token.h"
#include <string>
#include <vector>
 
class Lexer {
public:
    explicit Lexer(std::string source);
    std::vector<Token> tokenize();
 
private:
    std::string src_;
    std::size_t pos_  = 0;
    int         line_ = 1;
    int         col_  = 1;
 
    char peek(int offset = 0) const;
    char advance();
    void skip_whitespace_and_comments();
 
    Token make(TokenType t, std::string val) const;
    Token read_number();
    Token read_ident_or_keyword();
};