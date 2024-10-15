#pragma once
#include <string>
#include <unordered_set>

class Token;

class Tokenizer
{
    std::string::iterator it;
    std::string text;

    std::unordered_set<std::string> identifiers;
    std::unordered_set<int> consts;

public:
    explicit Tokenizer(std::string text);
    bool GetToken(Token& token);
    std::string message;

private:
    Token HandleKeywordOrIdentifier();
    Token HandleLiteral();
};