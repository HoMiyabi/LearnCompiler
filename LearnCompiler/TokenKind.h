#pragma once
#include <string_view>

enum class TokenKind
{
#define ENUM(x) x,
#include "TokenKind.def.h"
};

inline std::string_view ToString(TokenKind value){
    switch(value)
    {
#define ENUM(x) case TokenKind::x: return #x;
#include "TokenKind.def.h"
    default: return "Unknown";
    }
}