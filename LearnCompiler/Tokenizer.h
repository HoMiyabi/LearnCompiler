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
            return HandleKeywordOrIdentifier(token);
        }
        if (IsDigit(Current()))
        {
            return HandleLiteral(token);
        }
        if (startingPunctuators.contains(Current()))
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

    bool HandleKeywordOrIdentifier(Token& token)
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
            return true;
        }

        identifiers.insert(tokenStr);
        token.kind = TokenKind::Identifier;
        token.value = tokenStr;
        return true;
    }

    bool HandleLiteral(Token& token)
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
            message = errorPrefix + "数字" + tokenStr + "无法被解析";
            return false;
        }
        catch (std::out_of_range&)
        {
            message = errorPrefix + "数字" + tokenStr + "超出范围";
            return false;
        }
        consts.insert(value);

        token.kind = TokenKind::Int;
        token.value = value;
        return true;
    }

    bool HandlePunctuator(Token& token)
    {
        if (const auto it = singlePunctuatorSpellingToTokenKind.find(Current());
            it != singlePunctuatorSpellingToTokenKind.end())
        {
            token.kind = it->second;
            MoveNext();
            return true;
        }

        if (Current() == '<')
        {
            MoveNext();
            if (!IsEnd() && Current() == '>')
            {
                token.kind = TokenKind::LessGreater;
                MoveNext();
                return true;
            }
            if (!IsEnd() && Current() == '=')
            {
                token.kind = TokenKind::LessEqual;
                MoveNext();
                return true;
            }
            token.kind = TokenKind::Less;
            return true;
        }

        if (Current() == '>')
        {
            MoveNext();
            if (!IsEnd() && Current() == '=')
            {
                token.kind = TokenKind::GreaterEqual;
                MoveNext();
                return true;
            }
            token.kind = TokenKind::Greater;
            return true;
        }

        const std::string errorPrefix = GetErrorPrefix();
        if (Current() == ':')
        {
            MoveNext();
            if (!IsEnd() && Current() == '=')
            {
                token.kind = TokenKind::ColonEqual;
                MoveNext();
                return true;
            }
            message = errorPrefix + "未定义的标点符:";
            if (!IsEnd())
            {
                message += Current();
            }
            return false;
        }

        message = errorPrefix + "未定义的标点符" + Current();
        return false;
    }
};
