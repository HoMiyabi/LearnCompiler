#pragma once
#include <iostream>

#include "Tokenizer.h"

class Parser
{
private:
    Tokenizer& tokenizer;
    Token token{};
    int i = 0;
public:
    explicit Parser(Tokenizer& tokenizer):
    tokenizer(tokenizer)
    {
    }

    bool Parse()
    {
        Prog();
        return true;
    }

    void Match(TokenKind kind)
    {
        if (token.kind == kind)
        {
            tokenizer.GetToken(token);
        }
        else
        {
            throw std::runtime_error("Token不匹配");
        }
    }

    void MoveNext()
    {
        tokenizer.GetToken(token);
    }

    TokenKind CurrentKind() const
    {
        return token.kind;
    }

    void Prog()
    {
        Match(TokenKind::Program);
        Match(TokenKind::Identifier);
        Match(TokenKind::Semi);
        Block();
    }

    void Block()
    {
        if (CurrentKind() == TokenKind::Const)
        {
            Condecl();
        }
        if (CurrentKind() == TokenKind::Var)
        {
            Vardecl();
        }
        if (CurrentKind() == TokenKind::Procedure)
        {
            Proc();
        }
        Body();
    }


    void Condecl()
    {
        Match(TokenKind::Const);
        Const();
        while (CurrentKind() == TokenKind::Comma)
        {
            MoveNext();
            Const();
        }
    }

    void Const()
    {
        Match(TokenKind::Identifier);
        Match(TokenKind::ColonEqual);
        Match(TokenKind::Int);
    }

    void Vardecl()
    {
        Match(TokenKind::Var);
        Match(TokenKind::Identifier);
        while (CurrentKind() == TokenKind::Comma)
        {
            MoveNext();
            Match(TokenKind::Identifier);
        }
    }

    // Procedure
    void Proc()
    {
        Match(TokenKind::Procedure);
        Match(TokenKind::Identifier);
        Match(TokenKind::LParen);
        if (CurrentKind() == TokenKind::Identifier)
        {
            Match(TokenKind::Identifier);
            while (CurrentKind() == TokenKind::Comma)
            {
                MoveNext();
                Match(TokenKind::Identifier);
            }
        }
        Match(TokenKind::RParen);
        Match(TokenKind::Semi);
        Block();
        if (CurrentKind() == TokenKind::Semi)
        {
            MoveNext();
            Proc();
        }
    }

    void Body()
    {
        Match(TokenKind::Begin);
        Statement();
        while (CurrentKind() == TokenKind::Semi)
        {
            MoveNext();
            Statement();
        }
        Match(TokenKind::End);
    }

    void Statement()
    {
        switch (CurrentKind())
        {
        case TokenKind::Identifier:
            {
                Match(TokenKind::Identifier);
                Match(TokenKind::ColonEqual);
                Exp();
                break;
            }
        case TokenKind::If:
            {
                Match(TokenKind::If);
                Lexp();
                Match(TokenKind::Then);
                Statement();
                if (CurrentKind() == TokenKind::Else)
                {
                    MoveNext();
                    Statement();
                }
                break;
            }
        case TokenKind::While:
            {
                Match(TokenKind::While);
                Lexp();
                Match(TokenKind::Do);
                Statement();
                break;
            }
        case TokenKind::Call:
            {
                Match(TokenKind::Call);
                Match(TokenKind::Identifier);
                if (CurrentKind() == TokenKind::LParen)
                {
                    MoveNext();
                    Exp();
                    while (CurrentKind() == TokenKind::Comma)
                    {
                        MoveNext();
                        Exp();
                    }
                    Match(TokenKind::RParen);
                }
                break;
            }
        case TokenKind::Begin:
            {
                Body();
                break;
            }
        case TokenKind::Read:
            {
                Match(TokenKind::Read);
                Match(TokenKind::LParen);
                Match(TokenKind::Identifier);
                while (CurrentKind() == TokenKind::Comma)
                {
                    MoveNext();
                    Match(TokenKind::Identifier);
                }
                Match(TokenKind::RParen);
                break;
            }
        case TokenKind::Write:
            {
                Match(TokenKind::Write);
                Match(TokenKind::LParen);
                Exp();
                while (CurrentKind() == TokenKind::Comma)
                {
                    MoveNext();
                    Exp();
                }
                Match(TokenKind::RParen);
                break;
            }
        default:
            {
                throw std::runtime_error("Statement 错误");
            }
        }
    }

    // Logical Expression
    void Lexp()
    {

    }

    // Expression
    void Exp()
    {

    }

    void Term()
    {

    }

    void Factor()
    {
        switch (CurrentKind())
        {
        case TokenKind::Identifier:
            {
                Match(TokenKind::Identifier);
                break;
            }
        case TokenKind::Int:
            {
                Match(TokenKind::Int);
                break;
            }
        case TokenKind::LParen:
            {
                Match(TokenKind::LParen);
                Exp();
                Match(TokenKind::RParen);
                break;
            }
        default:
            {
                throw std::runtime_error("Factor 错误");
            }
        }
    }

    // Logical Operator
    void Lop()
    {
        switch (CurrentKind())
        {
        case TokenKind::Equal:
        case TokenKind::Less:
        case TokenKind::LessEqual:
        case TokenKind::Greater:
        case TokenKind::GreaterEqual:
            {
                MoveNext();
                break;
            }
        default:
            {
                throw std::runtime_error("Lop 错误");
            }
        }
    }

    void Aop()
    {
        if (CurrentKind() == TokenKind::Plus || CurrentKind() == TokenKind::Minus)
        {
            MoveNext();
        }
        else
        {
            throw std::runtime_error("Aop 错误");
        }
    }

    void Mop()
    {
        if (CurrentKind() == TokenKind::Star || CurrentKind() == TokenKind::Slash)
        {
            MoveNext();
        }
        else
        {
            throw std::runtime_error("Mop 错误");
        }
    }
};
