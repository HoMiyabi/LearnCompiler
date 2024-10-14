#pragma once
#include <string>
#include <unordered_set>

#include "Token.h"

class Tokenizer
{
    int i = 0;
    std::string text;

    std::unordered_set<std::string> identifiers;
    std::unordered_set<int> consts;

    explicit Tokenizer(const std::string& text);
    bool GetToken(Token& token);
    Token HandleKeywordOrIdentifier();
    Token HandleLiteral();
};
