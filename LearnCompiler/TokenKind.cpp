const char* tokenKindKeywordStrs[] =
{
#define TOKEN_KEYWORD(X) #X,
#include "TokenKind.def.h"
};