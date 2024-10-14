#include "Tokenizer.h"

Tokenizer::Tokenizer(const std::string& text): text(text)
{}

bool Tokenizer::GetToken(Token& token)
{
    const auto n = text.size();
    while (i < n)
    {
        if (isspace(text[i]))
        {
            i++;
        }
        else if (isalpha(text[i]))
        {
            token = HandleKeywordOrIdentifier();
            return true;
        }
        else if (isdigit(text[i]))
        {
            token = HandleLiteral();
            return true;
        }
        else if (text[i] == '+')
        {
            i++;
            token = Token(TokenKind::Add);
            return true;
        }
        else if (text[i] == '-')
        {
            i++;
            token = Token(TokenKind::Sub);
            return true;
        }
        else if (text[i] == '*')
        {
            i++;
            token = Token(TokenKind::Mul);
            return true;
        }
        else if (text[i] == '/')
        {
            i++;
            token = Token(TokenKind::Div);
            return true;
        }
        else if (text[i] == '=')
        {
            i++;
            token = Token(TokenKind::Assign);
            return true;
        }
    }
    return false;
}

Token Tokenizer::HandleKeywordOrIdentifier()
{
    int n = text.size();
    std::string tokenStr;
    do
    {
        tokenStr.push_back(text[i]);
        i++;
    } while (i < n && isalnum(text[i]));

    auto it = Token::strToKeywordKind.find(tokenStr);
    if (it != Token::strToKeywordKind.end())
    {
        return Token(it->second);
    }

    identifiers.insert(tokenStr);
    return Token(TokenKind::Identifier, tokenStr);
}

Token Tokenizer::HandleLiteral()
{
    int n = text.size();
    std::string tokenStr;
    do
    {
        tokenStr.push_back(text[i]);
        i++;
    } while (i < n && isdigit(text[i]));

    int value = std::stoi(tokenStr);

    consts.insert(value);
    return Token(TokenKind::Int, value);
}
