#include "Tokenizer.h"
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
        if (!isspace(*it))
        {
            break;
        }
        ++it;
    }

    if (isalpha(*it))
    {
        token = HandleKeywordOrIdentifier();
        return true;
    }
    if (isdigit(*it))
    {
        token = HandleLiteral();
        return true;
    }

    if (*it == '+')
    {
        ++it;
        token = Token(TokenKind::Add);
        return true;
    }
    if (*it == '-')
    {
        ++it;
        token = Token(TokenKind::Sub);
        return true;
    }
    if (*it == '*')
    {
        ++it;
        token = Token(TokenKind::Mul);
        return true;
    }
    if (*it == '/')
    {
        ++it;
        token = Token(TokenKind::Div);
        return true;
    }
    if (*it == ':')
    {
        char ch = *it;
        ++it;
        if (it == text.end())
        {
            message = "非预期的符号";
            message += ch;
            return false;
        }
        if (*it != '=')
        {
            message = "非预期的符号";
            message += ch;
            message += *it;
            --it;
            return false;
        }
        ++it;
        token = Token(TokenKind::Assign);
        return true;
    }
    if (*it == '=')
    {
        ++it;
        token = Token(TokenKind::Equal);
        return true;
    }
    if (*it == '<')
    {
        ++it;
        if (it == text.end() || *it != '=')
        {
            token = Token(TokenKind::Less);
            return true;
        }
        ++it;
        token = Token(TokenKind::LessEqual);
        return true;
    }
    if (*it == '>')
    {
        ++it;
        if (it == text.end() || *it != '=')
        {
            token = Token(TokenKind::Greater);
            return true;
        }
        ++it;
        token = Token(TokenKind::GreaterEqual);
        return true;
    }
    if (*it == '(')
    {
        ++it;
        token = Token(TokenKind::LParen);
        return true;
    }
    if (*it == ')')
    {
        ++it;
        token = Token(TokenKind::RParen);
        return true;
    }
    if (*it == '{')
    {
        ++it;
        token = Token(TokenKind::LBrace);
        return true;
    }
    if (*it == '}')
    {
        ++it;
        token = Token(TokenKind::RBrace);
        return true;
    }
    if (*it == ';')
    {
        ++it;
        token = Token(TokenKind::Semicolon);
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
    } while (it != text.end() && isalnum(*it));

    if (const auto its = Token::strToKeywordKind.find(tokenStr); its != Token::strToKeywordKind.end())
    {
        return Token(its->second);
    }

    identifiers.insert(tokenStr);
    return {TokenKind::Identifier, tokenStr};
}

Token Tokenizer::HandleLiteral()
{
    std::string tokenStr;
    do
    {
        tokenStr.push_back(*it);
        ++it;
    } while (it != text.end() && isdigit(*it));

    int value = std::stoi(tokenStr);

    consts.insert(value);
    return {TokenKind::Int, value};
}