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
        while (tokenizer.GetToken(token))
        {
            std::cout << token.ToString() << '\n';
        }
        if (!tokenizer.message.empty())
        {
            std::cout << tokenizer.message << '\n';
            return false;
        }
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
            throw std::runtime_error("Error");
        }
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
        Condecl();
        Vardecl();
        Proc();
    }


    void Condecl()
    {
        Match(TokenKind::Const);
        Const();
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
        // TODO
    }

    void Proc()
    {
        Match(TokenKind::Procedure);
        Match(TokenKind::Identifier);
        // TODO
    }

    void Body()
    {
        Match(TokenKind::Begin);
        Statement();
        Match(TokenKind::End);
    }

    void Statement()
    {
        // TODO
    }

    void Lexp()
    {

    }

    void Exp()
    {

    }

    void Term()
    {

    }

    void Factor()
    {

    }

    // Logical Operator
    void Lop()
    {

    }

    void Aop()
    {

    }

    void Mop()
    {

    }
};
