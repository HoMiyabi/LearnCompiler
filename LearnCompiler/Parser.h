#pragma once
#include <iostream>

#include "ILOP.h"
#include "Tokenizer.h"

enum class IdType
{
    Const,
    Var,
};

class IdInfo
{
public:
    IdType type;
    int32_t address;

    explicit IdInfo(IdType type, int32_t address):
    type(type),
    address(address)
    {
    }
};

class ProcedureInfo
{
public:
    int32_t address;
    std::string name;
    std::vector<std::string> params;

    ProcedureInfo(int32_t address, std::string name, std::vector<std::string> params):
    address(address),
    name(std::move(name)),
    params(std::move(params))
    {
    }
};

class Parser
{
private:
    Tokenizer& tokenizer;
    std::optional<Token> token;
    int i = 0;

    std::vector<ILOP> code;
    int stk = 0;
    std::unordered_map<std::string, IdInfo> idConstVar;
    std::unordered_map<std::string, ProcedureInfo> procedures;

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

    void GenCode(ILOP ilop)
    {
        code.push_back(ilop);
    }

    Token Match(const TokenKind kind)
    {
        if (!token.has_value())
        {
            throw std::runtime_error("预期的Token为" + std::string(magic_enum::enum_name(kind)) +
                    "， 但已经到达文件尾部");
        }

        Token tk = std::move(token.value());
        if (tk.kind != kind)
        {
            throw std::runtime_error(GetErrorPrefix(tk.fileLocation) +
            "预期的Token为" + std::string(magic_enum::enum_name(kind)) +
            "，但实际为" + std::string(magic_enum::enum_name(tk.kind)));
        }

        token = tokenizer.GetToken();
        return tk;
    }

    bool TryMatch(const TokenKind kind)
    {
        if (!token.has_value() || token.value().kind != kind)
        {
            return false;
        }

        token = tokenizer.GetToken();
        return true;
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
            throw std::runtime_error("预期下一个Token，但已经到达文件尾部");
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
        while (TryMatch(TokenKind::Comma))
        {
            Const();
        }
    }

    void Const()
    {
        auto tkId = Match(TokenKind::Identifier);
        Match(TokenKind::ColonEqual);
        auto tkInt = Match(TokenKind::Int);

        auto [it, ok] = idConstVar.insert({tkId.GetString(), IdInfo(IdType::Var, stk)});
        if (!ok)
        {
            if (it->second.type == IdType::Const)
            {
                throw std::runtime_error(GetErrorPrefix(tkId.fileLocation) + "已定义常量" + tkId.GetString());
            }
            if (it->second.type == IdType::Var)
            {
                throw std::runtime_error(GetErrorPrefix(tkId.fileLocation) + "已定义变量" + tkId.GetString());
            }
            throw std::runtime_error(GetErrorPrefix(tkId.fileLocation) + tkId.GetString() + "已定义");
        }

        GenCode(ILOP(ILOPCode::INT, 0, 1));
        GenCode(ILOP(ILOPCode::LIT, 0, tkInt.GetInt32()));
        GenCode(ILOP(ILOPCode::STO, 0, stk));
        stk++;
    }

    void Vardecl()
    {
        Match(TokenKind::Var);
        auto tkId = Match(TokenKind::Identifier);

        auto pair = idConstVar.insert({tkId.GetString(), IdInfo(IdType::Var, stk)});
        if (!pair.second)
        {
            std::string typeStr = pair.first->second.type == IdType::Const ? "常量" : "变量";
            throw std::runtime_error(GetErrorPrefix(tkId.fileLocation) + "已定义" + typeStr + tkId.GetString());
        }

        GenCode(ILOP(ILOPCode::INT, 0, 1));
        stk++;

        while (TryMatch(TokenKind::Comma))
        {
            tkId = Match(TokenKind::Identifier);

            pair = idConstVar.insert({tkId.GetString(), IdInfo(IdType::Var, stk)});
            if (!pair.second)
            {
                std::string typeStr = pair.first->second.type == IdType::Const ? "常量" : "变量";
                throw std::runtime_error(GetErrorPrefix(tkId.fileLocation) + "已定义" + typeStr + tkId.GetString());
            }

            GenCode(ILOP(ILOPCode::INT, 0, 1));
            stk++;
        }
    }

    // Procedure
    void Proc()
    {
        Match(TokenKind::Procedure);
        auto tkName = Match(TokenKind::Identifier);
        Match(TokenKind::LParen);

        std::vector<std::string> params;

        Token tkParam;
        if (CurrentKindIs(TokenKind::Identifier))
        {
            tkParam = Match(TokenKind::Identifier);
            params.push_back(tkParam.GetString());
            while (TryMatch(TokenKind::Comma))
            {
                tkParam = Match(TokenKind::Identifier);
                params.push_back(tkParam.GetString());
            }
        }
        Match(TokenKind::RParen);
        Match(TokenKind::Semi);

        procedures.insert({tkName.GetString(), ProcedureInfo(114514, tkName.GetString(), std::move(params))});

        Block();
        if (TryMatch(TokenKind::Semi))
        {
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
        if (TryMatch(TokenKind::Odd))
        {
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
