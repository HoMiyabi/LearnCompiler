#pragma once

#include <unordered_map>
#include <string>
#include <variant>

enum class TokenKind
{
    Begin, End, If, Then, While, Do, Var, Procedure, Call,
    Add, Sub, Mul, Div, Equal, NotEqual, Assign,
    Int,
    Identifier
};

class Token
{
public:
    TokenKind kind;
    std::variant<std::string, int> value;

    static std::unordered_map<std::string, TokenKind> strToKeywordKind;

    Token(TokenKind kind, std::variant<std::string, int>&& value);

    explicit Token(TokenKind kind);
};