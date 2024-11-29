#pragma once
#include <string>
#include <unordered_set>

#include "CharUtils.h"
#include "Token.h"
#include "TokenKind.h"

class Tokenizer
{
public:
    std::string message;
private:
    std::string text;
    std::string::iterator it;
    int row;
    int column;

public:
    explicit Tokenizer(std::string text): text(std::move(text)), row(1), column(1)
    {
        it = this->text.begin();
    }

    bool GetToken(Token& token)
    {
        while (true)
        {
            if (IsEnd())
            {
                return false;
            }
            if (!IsSpace(Current()))
            {
                break;
            }
            MoveNext();
        }

        if (IsLetter(Current()))
        {
            token = HandleKeywordOrIdentifier();
            return true;
        }
        if (IsDigit(Current()))
        {
            token = HandleLiteral();
            return true;
        }
        if (IsPunctuator(Current()))
        {
            token = HandlePunctuator();
            return true;
        }
        message = "非预期的字符'" + std::string{Current()} + "'位于" + std::to_string(row) + "行" + std::to_string(column) + "列";
        return false;
    }

private:
    std::unordered_set<std::string> identifiers;
    std::unordered_set<int> consts;

    char Current() const
    {
        return *it;
    }

    void MoveNext()
    {
        ++column;
        if (IsNewline(*it))
        {
            ++row;
            column = 0;
        }
        ++it;
    }

    bool IsEnd() const
    {
        return it == text.end();
    }

    Token HandleKeywordOrIdentifier()
    {
        std::string tokenStr;
        do
        {
            tokenStr.push_back(Current());
            MoveNext();
        } while (!IsEnd() && IsLetterOrDigit(Current()));

        if (const auto its = spellingToTokenKind.find(tokenStr); its != spellingToTokenKind.end())
        {
            return Token(its->second);
        }

        identifiers.insert(tokenStr);
        return {TokenKind::Identifier, tokenStr};
    }

    Token HandleLiteral()
    {
        std::string tokenStr;
        do
        {
            tokenStr.push_back(Current());
            MoveNext();
        } while (!IsEnd() && IsDigit(Current()));

        int value = std::stoi(tokenStr);

        consts.insert(value);
        return {TokenKind::Int, value};
    }

    Token HandlePunctuator()
    {
        std::string tokenStr;
        do
        {
            tokenStr.push_back(Current());
            MoveNext();
        } while (!IsEnd() && IsPunctuator(Current()));

        TokenKind tokenKind = spellingToTokenKind.at(tokenStr);
        return {tokenKind, tokenStr};
    }
};
