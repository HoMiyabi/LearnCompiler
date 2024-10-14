#include "Token.h"

std::unordered_map<std::string, TokenKind> Token::strToKeywordKind
{
    {"begin", TokenKind::Begin},
    {"end", TokenKind::End},
    {"if", TokenKind::If},
    {"then", TokenKind::Then},
    {"while", TokenKind::While},
    {"do", TokenKind::Do},
    {"var", TokenKind::Var},
    {"procedure", TokenKind::Procedure},
    {"call", TokenKind::Call}
};

Token::Token(const TokenKind kind, std::variant<std::string, int>&& value):
    kind(kind), value(std::move(value))
{
}

Token::Token(TokenKind kind): kind(kind)
{
}