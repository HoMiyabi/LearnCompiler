#include "Tokenizer.h"

#include "CharUtils.h"
#include "Token.h"
#include "TokenKind.h"

Tokenizer::Tokenizer(std::string text): text(std::move(text))
{
    it = this->text.begin();
}

bool Tokenizer::GetToken(Token& token)
{
    while (true)
    {
        if (it == text.end())
        {
            return false;
        }
        if (!IsSpace(*it))
        {
            break;
        }
        ++it;
    }

    if (IsLetter(*it))
    {
        token = HandleKeywordOrIdentifier();
        return true;
    }
    if (IsDigit(*it))
    {
        token = HandleLiteral();
        return true;
    }
    if (IsPunctuator(*it))
    {
        token = HandlePunctuator();
        return true;
    }
    message = "非预期的符号";
    message += *it;
    return false;
}

Token Tokenizer::HandleKeywordOrIdentifier()
{
    std::string tokenStr;
    do
    {
        tokenStr.push_back(*it);
        ++it;
    } while (it != text.end() && IsLetterOrDigit(*it));

    if (const auto its = spellingToTokenKind.find(tokenStr); its != spellingToTokenKind.end())
    {
        return Token(its->second);
    }

    identifiers.insert(tokenStr);
    return {TokenKind::Identifier, tokenStr};
}