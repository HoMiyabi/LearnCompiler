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
    std::string::iterator iterator;
    int row;
    int column;

public:
    explicit Tokenizer(std::string text): text(std::move(text)), row(1), column(1)
    {
        iterator = this->text.begin();
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
            HandleKeywordOrIdentifier(token);
            return true;
        }
        if (IsDigit(Current()))
        {
            HandleLiteral(token);
            return true;
        }
        if (startingPunctuators.contains(Current()))
        {
            HandlePunctuator(token);
            return true;
        }

        throw std::runtime_error(GetErrorPrefix() + "未定义的字符" + Current());
    }

private:
    std::unordered_set<std::string> identifiers;
    std::unordered_set<int> consts;

    char Current() const
    {
        return *iterator;
    }

    void MoveNext()
    {
        ++column;
        if (IsNewline(*iterator))
        {
            ++row;
            column = 1;
        }
        ++iterator;
    }

    bool IsEnd() const
    {
        return iterator == text.end();
    }

    std::string GetFileLocation() const
    {
        return std::to_string(row) + "行" + std::to_string(column) + "列";
    }

    std::string GetErrorPrefix() const
    {
        return "[错误] 位于" + GetFileLocation() + ": ";
    }

    void HandleKeywordOrIdentifier(Token& token)
    {
        std::string tokenStr;
        do
        {
            tokenStr.push_back(Current());
            MoveNext();
        } while (!IsEnd() && IsLetterOrDigit(Current()));

        if (const auto it = keywordSpellingToTokenKind.find(tokenStr); it != keywordSpellingToTokenKind.end())
        {
            token.kind = it->second;
            return;
        }

        identifiers.insert(tokenStr);
        token.kind = TokenKind::Identifier;
        token.value = tokenStr;
    }

    void HandleLiteral(Token& token)
    {
        const std::string errorPrefix = GetErrorPrefix();
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
            throw std::runtime_error(errorPrefix + "数字" + tokenStr + "无法被解析");
        }
        catch (std::out_of_range&)
        {
            throw std::runtime_error(errorPrefix + "数字" + tokenStr + "超出范围");
        }
        consts.insert(value);

        token.kind = TokenKind::Int;
        token.value = value;
    }

    void HandlePunctuator(Token& token)
    {
        if (const auto it = singlePunctuatorSpellingToTokenKind.find(Current());
            it != singlePunctuatorSpellingToTokenKind.end())
        {
            token.kind = it->second;
            MoveNext();
        }

        if (Current() == '<')
        {
            MoveNext();
            if (!IsEnd() && Current() == '>')
            {
                token.kind = TokenKind::LessGreater;
                MoveNext();
            }
            if (!IsEnd() && Current() == '=')
            {
                token.kind = TokenKind::LessEqual;
                MoveNext();
            }
            token.kind = TokenKind::Less;
        }

        if (Current() == '>')
        {
            MoveNext();
            if (!IsEnd() && Current() == '=')
            {
                token.kind = TokenKind::GreaterEqual;
                MoveNext();
            }
            token.kind = TokenKind::Greater;
        }

        const std::string errorPrefix = GetErrorPrefix();
        if (Current() == ':')
        {
            MoveNext();
            if (!IsEnd() && Current() == '=')
            {
                token.kind = TokenKind::ColonEqual;
                MoveNext();
            }
            throw std::runtime_error(errorPrefix + "未定义的标点符:");
        }

        throw std::runtime_error(errorPrefix + "未定义的标点符" + Current());
    }
};
