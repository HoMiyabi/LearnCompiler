#pragma once
#include <iostream>

#include "ILInst.h"
#include "ILInstOprType.h"
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
    int32_t codeAddress;
    std::string name;
    std::vector<std::string> params;

    ProcedureInfo(int32_t codeAddress, std::string name, std::vector<std::string> params):
    codeAddress(codeAddress),
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

    std::vector<ILInst> code;
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

    Token Match(const TokenKind kind)
    {
        if (!token)
        {
            throw std::runtime_error("预期的Token为" + std::string(magic_enum::enum_name(kind)) +
                    "， 但已经到达文件尾部");
        }

        Token tk = token.value();
        if (tk.kind != kind)
        {
            throw std::runtime_error(GetErrorPrefix(tk.fileLocation) +
            "预期的Token为" + std::string(magic_enum::enum_name(kind)) +
            "，但实际为" + std::string(magic_enum::enum_name(tk.kind)));
        }

        token = tokenizer.GetToken();
        return tk;
    }

    std::optional<Token> TryMatch(const TokenKind kind)
    {
        if (!token.has_value() || token.value().kind != kind)
        {
            return std::nullopt;
        }

        auto tk = token.value();

        token = tokenizer.GetToken();
        return tk;
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

        code.emplace_back(ILInstType::INT, 0, 1);
        code.emplace_back(ILInstType::LIT, 0, tkInt.GetInt32());
        code.emplace_back(ILInstType::STO, 0, stk);
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

        code.emplace_back(ILInstType::INT, 0, 1);
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

            code.emplace_back(ILInstType::INT, 0, 1);
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

        procedures.insert({tkName.GetString(), ProcedureInfo(code.size(), tkName.GetString(), std::move(params))});

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
        while (TryMatch(TokenKind::Semi))
        {
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
                if (TryMatch(TokenKind::Else))
                {
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
            code.emplace_back(ILInstType::OPR, 0, static_cast<int32_t>(ILInstOprType::Odd));
        }
        else
        {
            Exp();
            auto tk = Lop();
            Exp();
            switch (tk.kind)
            {
            case TokenKind::Equal:
                {
                    code.emplace_back(ILInstType::OPR, 0, static_cast<int32_t>(ILInstOprType::Eql));
                    break;
                }
            case TokenKind::LessGreater:
                {
                    code.emplace_back(ILInstType::OPR, 0, static_cast<int32_t>(ILInstOprType::Neq));
                    break;
                }
            case TokenKind::Less:
                {
                    code.emplace_back(ILInstType::OPR, 0, static_cast<int32_t>(ILInstOprType::Lss));
                    break;
                }
            case TokenKind::LessEqual:
                {
                    code.emplace_back(ILInstType::OPR, 0, static_cast<int32_t>(ILInstOprType::Leq));
                    break;
                }
            case TokenKind::Greater:
                {
                    code.emplace_back(ILInstType::OPR, 0, static_cast<int32_t>(ILInstOprType::Gtr));
                    break;
                }
            case TokenKind::GreaterEqual:
                {
                    code.emplace_back(ILInstType::OPR, 0, static_cast<int32_t>(ILInstOprType::Geq));
                    break;
                }
            default:
                {
                    throw std::runtime_error(GetErrorPrefix(tk.fileLocation) + "Lexp 错误");
                }
            }
        }
    }

    // exp.first = +, -, id, int, (
    // <exp> -> [+|-]<term>{<aop><term>}
    // Expression
    void Exp()
    {
        bool flag = false;
        if (TryMatch(TokenKind::Minus))
        {
            flag = true;
        }
        else
        {
            TryMatch(TokenKind::Plus);
        }
        Term();
        if (flag)
        {
            code.emplace_back(ILInstType::OPR, 0, static_cast<int32_t>(ILInstOprType::Neg));
        }
        while (token && (token->kind == TokenKind::Minus || token->kind == TokenKind::Plus))
        {
            Token tkOp = *token;
            MoveNext();
            Term();
            if (tkOp.kind == TokenKind::Minus)
            {
                code.emplace_back(ILInstType::OPR, 0, static_cast<int32_t>(ILInstOprType::Sub));
            }
            else
            {
                code.emplace_back(ILInstType::OPR, 0, static_cast<int32_t>(ILInstOprType::Add));
            }
        }
    }

    // term.first = id, int, (
    void Term()
    {
        Factor();
        while (token && (token->kind == TokenKind::Star || token->kind == TokenKind::Slash))
        {
            auto tkOp = *token;
            Factor();
            if (tkOp.kind == TokenKind::Star)
            {
                code.emplace_back(ILInstType::OPR, 0, static_cast<int32_t>(ILInstOprType::Mul));
            }
            else
            {
                code.emplace_back(ILInstType::OPR, 0, static_cast<int32_t>(ILInstOprType::Div));
            }
        }
    }
    // <factor> -> <id>|<integer>|(<exp>)
    // factor.first = id, int, (
    void Factor()
    {
        std::optional<Token> tk;
        tk = TryMatch(TokenKind::Identifier);
        if (tk)
        {
            auto it = idConstVar.find(tk->GetString());
            if (it == idConstVar.end())
            {
                throw std::runtime_error(GetErrorPrefix(tk->fileLocation) + "未定义的标识符");
            }
            code.emplace_back(ILInstType::LOD, 0, it->second.address);
        }
        tk = TryMatch(TokenKind::Int);
        if (tk)
        {
            code.emplace_back(ILInstType::LIT, 0, tk->GetInt32());
            return;
        }
        if (TryMatch(TokenKind::LParen))
        {
            Exp();
            Match(TokenKind::RParen);
            return;
        }
        throw std::runtime_error(GetErrorPrefix(Current().fileLocation) + "Factor 错误");
    }

    // Logical Operator
    Token Lop()
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
                return tk;
            }
        default:
            {
                throw std::runtime_error(GetErrorPrefix(tk.fileLocation) + "Lop 错误");
            }
        }
    }
};
