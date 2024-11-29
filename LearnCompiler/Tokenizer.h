#pragma once
#include <string>
#include <unordered_set>

#include "CharUtils.h"
#include "Token.h"
#include "TokenKind.h"

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

    Token HandleLiteral()
    {
        std::string tokenStr;
        do
        {
            tokenStr.push_back(*it);
            ++it;
        } while (it != text.end() && IsDigit(*it));

        int value = std::stoi(tokenStr);

        consts.insert(value);
        return {TokenKind::Int, value};
    }

    Token HandlePunctuator()
    {
        std::string tokenStr;
        do
        {
            tokenStr.push_back(*it);
            ++it;
        } while (it != text.end() && IsPunctuator(*it));

        TokenKind tokenKind = spellingToTokenKind.at(tokenStr);
        return {tokenKind, tokenStr};
    }
};
