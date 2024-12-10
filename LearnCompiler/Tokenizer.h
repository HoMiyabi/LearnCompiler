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
            return HandleKeywordOrIdentifier(token);
        }
        if (IsDigit(Current()))
        {
            return HandleLiteral(token);
        }
        if (IsPunctuator(Current()))
        {
            return HandlePunctuator(token);
        }
        message = GetErrorPrefix() + "非预期的字符" + std::string{Current()};
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
            column = 1;
        }
        ++it;
    }

    bool IsEnd() const
    {
        return it == text.end();
    }

    std::string GetFileLocation() const
    {
        return std::to_string(row) + "行" + std::to_string(column) + "列";
    }

    std::string GetErrorPrefix() const
    {
        return "[错误] 位于" + GetFileLocation() + ": ";
    }

    bool HandleKeywordOrIdentifier(Token& token)
    {
        std::string tokenStr;
        do
        {
            tokenStr.push_back(Current());
            MoveNext();
        } while (!IsEnd() && IsLetterOrDigit(Current()));

        if (const auto its = spellingToTokenKind.find(tokenStr); its != spellingToTokenKind.end())
        {
            token.kind = its->second;
            return true;
        }

        identifiers.insert(tokenStr);
        token.kind = TokenKind::Identifier;
        token.value = tokenStr;
        return true;
    }

    bool HandleLiteral(Token& token)
    {
        std::string tokenStr;
        do
        {
            tokenStr.push_back(Current());
            MoveNext();
        } while (!IsEnd() && IsDigit(Current()));

        int value;
        try
        {
            value = std::stoi(tokenStr);
        }
        catch (std::invalid_argument&)
        {
            message = GetErrorPrefix() + "数字" + tokenStr + "无法被解析";
            return false;
        }
        catch (std::out_of_range&)
        {
            message = GetErrorPrefix() + "数字" + tokenStr + "超出范围";
            return false;
        }
        consts.insert(value);

        token.kind = TokenKind::Int;
        token.value = value;
        return true;
    }

    bool HandlePunctuator(Token& token)
    {
        std::string tokenStr;
        do
        {
            tokenStr.push_back(Current());
            MoveNext();
        } while (!IsEnd() && IsPunctuator(Current()));

        auto it1 = spellingToTokenKind.find(tokenStr);
        if (it1 == spellingToTokenKind.end())
        {
            message = GetErrorPrefix() + "标点符" + tokenStr + "未定义";
            return false;
        }

        token.kind = it1->second;
        token.value = tokenStr;
        return true;
    }
};
