#pragma once
#include <string_view>

enum TokenKind
{
#define TOKEN_KEYWORD(X) kw_##X,
#include "TokenKind.def.h"
};

enum TokenKeywordKind
{
#define TOKEN_KEYWORD(X) kw_##X,
#include "TokenKind.def.h"
};