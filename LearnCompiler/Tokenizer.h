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

    std::optional<Token> GetToken()
    {
        while (true)
        {
            if (IsEnd())
            {
                return std::nullopt;
            }
            if (!IsSpace(Current()))
            {
                break;
            }
            MoveNext();
        }

        if (IsLetter(Current()))
        {
            return HandleKeywordOrIdentifier();
        }
        if (IsDigit(Current()))
        {
            return HandleLiteral();
        }
        if (startingPunctuators.contains(Current()))
        {
            return HandlePunctuator();
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
        return "[词法错误] 位于" + GetFileLocation() + ": ";
    }

    Token HandleKeywordOrIdentifier()
    {
        std::string tokenStr;
        do
        {
            tokenStr.push_back(Current());
            MoveNext();
        } while (!IsEnd() && IsLetterOrDigit(Current()));

        if (const auto it = keywordSpellingToTokenKind.find(tokenStr); it != keywordSpellingToTokenKind.end())
        {
            return Token(it->second);
        }

        identifiers.insert(tokenStr);
        return {TokenKind::Identifier, tokenStr};
    }

    Token HandleLiteral()
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

        return {TokenKind::Int, value};
    }

    Token HandlePunctuator()
    {
        if (const auto it = singlePunctuatorSpellingToTokenKind.find(Current());
            it != singlePunctuatorSpellingToTokenKind.end())
        {
            MoveNext();
            return Token(it->second);
        }

        if (Current() == '<')
        {
            MoveNext();
            if (!IsEnd() && Current() == '>')
            {
                MoveNext();
                return Token(TokenKind::LessGreater);
            }
            if (!IsEnd() && Current() == '=')
            {
                MoveNext();
                return Token(TokenKind::LessEqual);
            }
            return Token(TokenKind::Less);
        }

        if (Current() == '>')
        {
            MoveNext();
            if (!IsEnd() && Current() == '=')
            {
                MoveNext();
                return Token(TokenKind::GreaterEqual);
            }
            return Token(TokenKind::Greater);
        }

        const std::string errorPrefix = GetErrorPrefix();
        if (Current() == ':')
        {
            MoveNext();
            if (!IsEnd() && Current() == '=')
            {
                MoveNext();
                return Token(TokenKind::ColonEqual);
            }
            throw std::runtime_error(errorPrefix + "未定义的标点符:");
        }

        throw std::runtime_error(errorPrefix + "未定义的标点符" + Current());
    }
};
