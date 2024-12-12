#pragma once
#include <string>
#include <unordered_set>

#include "CharUtils.h"
#include "FileLocation.h"
#include "Token.h"
#include "TokenKind.h"

class Tokenizer
{
public:
    std::string message;
private:
    std::string text;
    std::string::iterator iterator;
    FileLocation fileLocation;

public:
    explicit Tokenizer(std::string text): text(std::move(text))
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

        throw std::runtime_error(GetErrorPrefix(fileLocation) + "未定义的字符" + Current());
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
        if (IsNewline(*iterator))
        {
            fileLocation.ToNewline();
        }
        else
        {
            ++fileLocation.column;
        }
        ++iterator;
    }

    bool IsEnd() const
    {
        return iterator == text.end();
    }

    std::string static GetErrorPrefix(const FileLocation location)
    {
        return "[词法错误] 位于" + location.ToString() + ": ";
    }

    Token HandleKeywordOrIdentifier()
    {
        const auto location = fileLocation;

        std::string tokenStr;
        do
        {
            tokenStr.push_back(Current());
            MoveNext();
        } while (!IsEnd() && IsLetterOrDigit(Current()));

        if (const auto it = keywordSpellingToTokenKind.find(tokenStr); it != keywordSpellingToTokenKind.end())
        {
            return {location, it->second};
        }

        identifiers.insert(tokenStr);
        return {location, TokenKind::Identifier, tokenStr};
    }

    Token HandleLiteral()
    {
        const auto location = fileLocation;

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
            throw std::runtime_error(GetErrorPrefix(location) + "数字" + tokenStr + "无法被解析");
        }
        catch (std::out_of_range&)
        {
            throw std::runtime_error(GetErrorPrefix(location) + "数字" + tokenStr + "超出范围");
        }
        consts.insert(value);

        return {location, TokenKind::Int, value};
    }

    Token HandlePunctuator()
    {
        const auto location = fileLocation;

        if (const auto it = singlePunctuatorSpellingToTokenKind.find(Current());
            it != singlePunctuatorSpellingToTokenKind.end())
        {
            MoveNext();
            return {location, it->second};
        }

        if (Current() == '<')
        {
            MoveNext();
            if (!IsEnd() && Current() == '>')
            {
                MoveNext();
                return {location, TokenKind::LessGreater};
            }
            if (!IsEnd() && Current() == '=')
            {
                MoveNext();
                return {location, TokenKind::LessEqual};
            }
            return {location, TokenKind::Less};
        }

        if (Current() == '>')
        {
            MoveNext();
            if (!IsEnd() && Current() == '=')
            {
                MoveNext();
                return {location, TokenKind::GreaterEqual};
            }
            return {location, TokenKind::Greater};
        }

        if (Current() == ':')
        {
            MoveNext();
            if (!IsEnd() && Current() == '=')
            {
                MoveNext();
                return {location, TokenKind::ColonEqual};
            }
            throw std::runtime_error(GetErrorPrefix(location) + "未定义的标点符:");
        }

        throw std::runtime_error(GetErrorPrefix(location) + "未定义的标点符" + Current());
    }
};
