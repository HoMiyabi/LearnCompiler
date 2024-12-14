#pragma once
#include <string>
#include <variant>

#include "TokenKind.h"
#include "include/magic_enum/magic_enum.hpp"

class Token
{
public:
    TokenKind kind;
    std::variant<std::string, int32_t> value;
    FileLocation fileLocation;

    Token() = default;

    Token(const FileLocation fileLocation, const TokenKind kind): kind(kind), fileLocation(fileLocation)
    {
    }

    Token(const FileLocation fileLocation, const TokenKind kind, std::variant<std::string, int> value):
    kind(kind), value(std::move(value)), fileLocation(fileLocation)
    {
    }

    [[nodiscard]]
    const std::string& String() const
    {
        return std::get<std::string>(value);
    }

    [[nodiscard]]
    int32_t Int32() const
    {
        return std::get<int32_t>(value);
    }

    std::string ToString() const
    {
        std::string text;
        text += magic_enum::enum_name(kind);
        if (kind == TokenKind::Int)
        {
            text += ' ';
            text += std::to_string(std::get<int32_t>(value));
        }
        else if (kind == TokenKind::Identifier)
        {
            text += ' ';
            text += std::get<std::string>(value);
        }
        return text;
    }
};
