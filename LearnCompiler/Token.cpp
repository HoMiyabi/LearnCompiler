#include "Token.h"
#include "TokenKind.h"

Token::Token(const TokenKind kind, std::variant<std::string, int> value):
    kind(kind), value(std::move(value))
{
}

Token::Token() = default;

Token::Token(TokenKind kind): kind(kind)
{
}

std::string Token::ToString() const
{
    std::string text;
    text += GetSpelling(kind);
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