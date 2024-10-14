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
    explicit Token(TokenKind kind);
    Token(TokenKind kind, std::variant<std::string, int> value);

    std::string ToString() const;
};