#pragma once

#include <unordered_map>
#include <string>
#include <variant>

enum class TokenKind;

class Token
{
public:
    TokenKind kind;
    std::variant<std::string, int> value;

    static std::unordered_map<std::string, TokenKind> strToKeywordKind;

    Token();
    Token(TokenKind kind, std::variant<std::string, int> value);

    explicit Token(TokenKind kind);

    std::string ToString() const;
};