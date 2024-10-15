#include "Tokenizer.h"
#include "Token.h"
#include "TokenKind.h"

Tokenizer::Tokenizer(std::string text): text(std::move(text))
{
}

bool Tokenizer::GetToken(Token& token)
{
    char ch = 0;
    while (!IsEnd())
    {
        ch = Current();
        if (!isspace(ch))
        {
            break;
        }
        MoveNext();
    }
    if (IsEnd()) return false;

    if (isalpha(ch))
    {
        token = HandleKeywordOrIdentifier();
        return true;
    }
    if (isdigit(ch))
    {
        token = HandleLiteral();
        return true;
    }
    if (ch == '+')
    {
        MoveNext();
        token = Token(TokenKind::Add);
        return true;
    }
    if (ch == '-')
    {
        MoveNext();
        token = Token(TokenKind::Sub);
        return true;
    }
    if (ch == '*')
    {
        MoveNext();
        token = Token(TokenKind::Mul);
        return true;
    }
    if (ch == '/')
    {
        MoveNext();
        token = Token(TokenKind::Div);
        return true;
    }
    if (ch == ':')
    {
        MoveNext();
        if (IsEnd())
        {
            message = "非预期的符号";
            message += ch;
            return false;
        }
        if (Current() != '=')
        {
            message = "非预期的符号";
            message += ch;
            message += Current();
            MovePrev();
            return false;
        }
        MoveNext();
        token = Token(TokenKind::Assign);
        return true;
    }
    if (ch == '=')
    {
        MoveNext();
        token = Token(TokenKind::Equal);
        return true;
    }
    if (ch == '<')
    {
        MoveNext();
        if (IsEnd() || Current() != '=')
        {
            token = Token(TokenKind::Less);
            return true;
        }
        MoveNext();
        token = Token(TokenKind::LessOrEqual);
        return true;
    }
    if (ch == '>')
    {
        MoveNext();
        if (IsEnd() || Current() != '=')
        {
            token = Token(TokenKind::Greater);
            return true;
        }
        MoveNext();
        token = Token(TokenKind::GreaterOrEqual);
        return true;
    }
    if (ch == '(')
    {
        MoveNext();
        token = Token(TokenKind::LParen);
        return true;
    }
    if (ch == ')')
    {
        MoveNext();
        token = Token(TokenKind::RParen);
        return true;
    }
    if (ch == '{')
    {
        MoveNext();
        token = Token(TokenKind::LBrace);
        return true;
    }
    if (ch == '}')
    {
        MoveNext();
        token = Token(TokenKind::RBrace);
        return true;
    }
    if (ch == ';')
    {
        MoveNext();
        token = Token(TokenKind::Semicolon);
        return true;
    }
    message = "非预期的符号";
    message += ch;
    return false;
}

Token Tokenizer::HandleKeywordOrIdentifier()
{
    std::string tokenStr;
    do
    {
        tokenStr.push_back(Current());
        MoveNext();
    } while (!IsEnd() && isalnum(Current()));

    auto it = Token::strToKeywordKind.find(tokenStr);
    if (Token::strToKeywordKind.contains(tokenStr))
    {
        return Token(it->second);
    }

    identifiers.insert(tokenStr);
    return Token(TokenKind::Identifier, tokenStr);
}

Token Tokenizer::HandleLiteral()
{
    std::string tokenStr;
    do
    {
        tokenStr.push_back(Current());
        MoveNext();
    } while (!IsEnd() && isdigit(Current()));

    int value = std::stoi(tokenStr);

    consts.insert(value);
    return Token(TokenKind::Int, value);
}

bool Tokenizer::IsEnd() const
{
    return i == text.size();
}

void Tokenizer::MoveNext()
{
    i++;
}

void Tokenizer::MovePrev()
{
    i--;
}

char Tokenizer::Current() const
{
    return text[i];
}
