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
        ProcessBOM();
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
        if (IsPlusOrMinus(Current()) || IsDigit(Current()))
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
    void ProcessBOM()
    {
        if (text.size() >= 3 && text[0] == '\xEF' && text[1] == '\xBB' && text[2] == '\xBF')
        {
            iterator = text.begin() + 3;
        }
    }

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

        return {location, TokenKind::Identifier, tokenStr};
    }

    Token HandleLiteral()
    {
        const auto location = fileLocation;

        bool bFloat = false;

        std::string tokenStr;
        do
        {
            if (Current() == '.')
            {
                bFloat = true;
            }
            tokenStr.push_back(Current());
            MoveNext();
        } while (!IsEnd() && IsDigit(Current()));

        if (bFloat)
        {
            float f;
            try
            {
                f = std::stof(tokenStr);
            }
            catch (std::invalid_argument&)
            {
                throw std::runtime_error(GetErrorPrefix(location) + "字面量" + tokenStr + "无法被解析");
            }
            catch (std::out_of_range&)
            {
                throw std::runtime_error(GetErrorPrefix(location) + "字面量" + tokenStr + "超出范围");
            }
            return {location, TokenKind::Int32, f};
        }
        int i;
        try
        {
            i = std::stoi(tokenStr);
        }
        catch (std::invalid_argument&)
        {
            throw std::runtime_error(GetErrorPrefix(location) + "字面量" + tokenStr + "无法被解析");
        }
        catch (std::out_of_range&)
        {
            throw std::runtime_error(GetErrorPrefix(location) + "字面量" + tokenStr + "超出范围");
        }
        return {location, TokenKind::Int32, i};
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
            return {location, TokenKind::Colon};
        }

        throw std::runtime_error(GetErrorPrefix(location) + "未定义的标点符" + Current());
    }
};
