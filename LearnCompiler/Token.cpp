#include "Token.h"
#include "TokenKind.h"

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

Token::Token(const TokenKind kind, std::variant<std::string, int> value):
    kind(kind), value(std::move(value))
{
}

Token::Token()
{
}

Token::Token(TokenKind kind): kind(kind)
{
}

std::string Token::ToString() const
{
    std::string text;
    text += TokenKindToString(kind);
    if (kind == TokenKind::Int)
    {
        text += ' ';
        text += std::to_string(std::get<int>(value));
    }
    else if (kind == TokenKind::Identifier)
    {
        text += ' ';
        text += std::get<std::string>(value);
    }
    return text;
}
