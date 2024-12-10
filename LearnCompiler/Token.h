#pragma once
#include <string>
#include <variant>

#include "TokenKind.h"
#include "include/magic_enum/magic_enum.hpp"

class Token
{
public:
    TokenKind kind;
    std::variant<std::string, int> value;

    Token() = default;

    explicit Token(const TokenKind kind): kind(kind)
    {
    }

    Token(const TokenKind kind, std::variant<std::string, int> value): kind(kind), value(std::move(value))
    {
    }

    std::string ToString() const
    {
        std::string text;
        text += magic_enum::enum_name(kind);
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
};
