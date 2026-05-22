#pragma once
#include <string>
 
enum class TokenType {
    // Literals
    INT_LIT, FLOAT_LIT, BOOL_LIT,
 
    // Identifiers
    IDENT,
 
    // Keywords
    KW_FN, KW_LET, KW_RETURN,
    KW_IF, KW_ELSE,
    KW_TRUE, KW_FALSE,
 
    // Types
    TY_INT, TY_FLOAT, TY_BOOL,
 
    // Operators
    PLUS, MINUS, STAR, SLASH,
    EQ, EQ_EQ, BANG_EQ,
    LT, GT, LT_EQ, GT_EQ,
    AND_AND, OR_OR, BANG,
    ARROW,       // ->
    COLON,       // :
    COMMA,       // ,
    SEMICOLON,   // ;
 
    // Delimiters
    LPAREN, RPAREN,
    LBRACE, RBRACE,
 
    END_OF_FILE
};
 
struct Token {
    TokenType   type;
    std::string value;
    int         line;
    int         col;
};
 