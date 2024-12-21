module;
#include <optional>
#include <stdexcept>
#include <string>

export module Tokenizer;
import FileLocation;
import Token;
import CharUtils;
import TokenKind;
import ErrorUtils;

export class Tokenizer
{
public:
    std::string filePath;
    std::string text;
    std::string::iterator iterator;
    FileLocation fileLocation;

public:
    explicit Tokenizer(std::string filePath, std::string text):
    filePath(filePath), text(std::move(text))
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
        if (IsDigit(Current()))
        {
            return HandleLiteral();
        }
        if (IsPlusOrMinus(Current()))
        {
            auto next = iterator + 1;
            if (next != text.end() && IsDigit(*next))
            {
                return HandleLiteral();
            }
        }
        if (startingPunctuators.contains(Current()))
        {
            return HandlePunctuator();
        }
        ThrowLexical(filePath, fileLocation, "未定义的字符" + Current());
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
            return {location, it->second, std::move(tokenStr), filePath};
        }

        return {location, TokenKind::Identifier, std::move(tokenStr), filePath};
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
        } while (!IsEnd() && IsNumeric(Current()));

        try
        {
            if (bFloat)
            {
                float f = std::stof(tokenStr);
                return {location, TokenKind::Float32, f, std::move(tokenStr), filePath};
            }
            int i = std::stoi(tokenStr);
            return {location, TokenKind::Int32, i, std::move(tokenStr), filePath};
        }
        catch (std::invalid_argument&)
        {
            ThrowLexical(filePath, location, "字面量" + tokenStr + "无法被解析");
        }
        catch (std::out_of_range&)
        {
            ThrowLexical(filePath, location, "字面量" + tokenStr + "超出范围");
        }
    }

    Token HandlePunctuator()
    {
        const auto location = fileLocation;

        if (const auto it = singlePunctuatorSpellingToTokenKind.find(Current());
            it != singlePunctuatorSpellingToTokenKind.end())
        {
            MoveNext();
            return {location, it->second, std::string{it->first}, filePath};
        }

        if (Current() == '<')
        {
            MoveNext();
            if (!IsEnd() && Current() == '>')
            {
                MoveNext();
                return {location, TokenKind::LessGreater, "<>", filePath};
            }
            if (!IsEnd() && Current() == '=')
            {
                MoveNext();
                return {location, TokenKind::LessEqual, "<=", filePath};
            }
            return {location, TokenKind::Less, "<", filePath};
        }

        if (Current() == '>')
        {
            MoveNext();
            if (!IsEnd() && Current() == '=')
            {
                MoveNext();
                return {location, TokenKind::GreaterEqual, ">=", filePath};
            }
            return {location, TokenKind::Greater, ">", filePath};
        }

        if (Current() == ':')
        {
            MoveNext();
            if (!IsEnd() && Current() == '=')
            {
                MoveNext();
                return {location, TokenKind::ColonEqual, ":=", filePath};
            }
            return {location, TokenKind::Colon, ":", filePath};
        }

        ThrowLexical(filePath, location, "未定义的标点符" + Current());
    }
};
