#pragma once
#include <string_view>

enum class TokenKind
{
#define TOKEN_KIND_ENUM(x) x,
#include "TokenKind.def.h"
};

inline std::string_view TokenKindToString(TokenKind value){
    switch(value)
    {
#define TOKEN_KIND_ENUM(x) case TokenKind::x: return #x;
#include "TokenKind.def.h"
    default: return "Unknown";
    }
}