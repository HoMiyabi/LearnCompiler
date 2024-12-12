#pragma once
#include <iostream>

#include "Tokenizer.h"

class Parser
{
private:
    Tokenizer& tokenizer;
    std::optional<Token> token;
    int i = 0;
public:
    explicit Parser(Tokenizer& tokenizer):
    tokenizer(tokenizer)
    {
    }

    bool Parse()
    {
        token = tokenizer.GetToken();
        try
        {
            Prog();
        }
        catch (const std::exception& e)
        {
            std::cout << e.what() << '\n';
        }
        return true;
    }

private:
    std::string static GetErrorPrefix(const FileLocation location)
    {
        return "[语法错误] 位于" + location.ToString() + ": ";
    }

    void Match(const TokenKind kind)
    {
        if (!token.has_value())
        {
            throw std::runtime_error("预期的Token为" + std::string(magic_enum::enum_name(kind)) +
                    "， 但已经到达文件尾部");
        }

        if (const Token tk(std::move(token.value()));
            tk.kind != kind)
        {
            throw std::runtime_error(GetErrorPrefix(tk.fileLocation) +
                "预期的Token为" + std::string(magic_enum::enum_name(kind)) +
                "，但实际为" + std::string(magic_enum::enum_name(tk.kind)));
        }

        token = tokenizer.GetToken();
    }

    [[nodiscard]]
    bool CurrentKindIs(const TokenKind kind) const
    {
        if (!token.has_value())
        {
            throw std::runtime_error("预期的Token为" + std::string(magic_enum::enum_name(kind)) +
                    "，但已经到达文件尾部");
        }

        return token.value().kind == kind;
    }

    Token Current()
    {
        if (!token.has_value())
        {
            throw std::runtime_error("预期Token，但已经到达文件尾部");
        }
        return token.value();
    }

    void MoveNext()
    {
        token = tokenizer.GetToken();
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
        if (CurrentKindIs(TokenKind::Const))
        {
            Condecl();
        }
        if (CurrentKindIs(TokenKind::Var))
        {
            Vardecl();
        }
        if (CurrentKindIs(TokenKind::Procedure))
        {
            Proc();
        }
        Body();
    }


    void Condecl()
    {
        Match(TokenKind::Const);
        Const();
        while (CurrentKindIs(TokenKind::Comma))
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
        while (CurrentKindIs(TokenKind::Comma))
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
        if (CurrentKindIs(TokenKind::Identifier))
        {
            Match(TokenKind::Identifier);
            while (CurrentKindIs(TokenKind::Comma))
            {
                MoveNext();
                Match(TokenKind::Identifier);
            }
        }
        Match(TokenKind::RParen);
        Match(TokenKind::Semi);
        Block();
        if (CurrentKindIs(TokenKind::Semi))
        {
            MoveNext();
            Proc();
        }
    }

    void Body()
    {
        Match(TokenKind::Begin);
        Statement();
        while (CurrentKindIs(TokenKind::Semi))
        {
            MoveNext();
            Statement();
        }
        Match(TokenKind::End);
    }

    void Statement()
    {

        switch (Current().kind)
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
                if (CurrentKindIs(TokenKind::Else))
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
                if (CurrentKindIs(TokenKind::LParen))
                {
                    MoveNext();
                    Exp();
                    while (CurrentKindIs(TokenKind::Comma))
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
                while (CurrentKindIs(TokenKind::Comma))
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
                while (CurrentKindIs(TokenKind::Comma))
                {
                    MoveNext();
                    Exp();
                }
                Match(TokenKind::RParen);
                break;
            }
        default:
            {
                throw std::runtime_error(GetErrorPrefix(Current().fileLocation) + "Statement 错误");
            }
        }
    }

    // <lexp> → <exp> <lop> <exp>|odd <exp>
    // Logical Expression
    void Lexp()
    {
        if (CurrentKindIs(TokenKind::Odd))
        {
            MoveNext();
            Exp();
        }
        else
        {
            Exp();
            Lop();
            Exp();
        }
    }

    // exp.first = +, -, id, int, (
    // <exp> -> [+|-]<term>{<aop><term>}
    // Expression
    void Exp()
    {
        if (CurrentKindIs(TokenKind::Plus) || CurrentKindIs(TokenKind::Minus))
        {
            MoveNext();
        }
        Term();
        while (CurrentKindIs(TokenKind::Plus) || CurrentKindIs(TokenKind::Minus))
        {
            Aop();
            Term();
        }
    }

    // term.first = id, int, (
    void Term()
    {
        Factor();
        while (CurrentKindIs(TokenKind::Star) || CurrentKindIs(TokenKind::Slash))
        {
            Mop();
            Factor();
        }
    }

    // factor.first = id, int, (
    void Factor()
    {
        switch (Current().kind)
        {
        case TokenKind::Identifier:
            {
                MoveNext();
                break;
            }
        case TokenKind::Int:
            {
                MoveNext();
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
                throw std::runtime_error(GetErrorPrefix(Current().fileLocation) +"Factor 错误");
            }
        }
    }

    // Logical Operator
    void Lop()
    {
        const auto tk = Current();
        switch (tk.kind)
        {
        case TokenKind::Equal:
        case TokenKind::LessGreater:
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
                throw std::runtime_error(GetErrorPrefix(tk.fileLocation) + "Lop 错误");
            }
        }
    }

    void Aop()
    {
        if (CurrentKindIs(TokenKind::Plus) || CurrentKindIs(TokenKind::Minus))
        {
            MoveNext();
        }
        else
        {
            throw std::runtime_error(GetErrorPrefix(Current().fileLocation) + "Aop 错误");
        }
    }

    void Mop()
    {
        if (CurrentKindIs(TokenKind::Star) || CurrentKindIs(TokenKind::Slash))
        {
            MoveNext();
        }
        else
        {
            throw std::runtime_error(GetErrorPrefix(Current().fileLocation) + "Mop 错误");
        }
    }
};
