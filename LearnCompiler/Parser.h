#pragma once
#include <iostream>

#include "ILInst.h"
#include "ILInstOprType.h"
#include "Tokenizer.h"

enum class VarType
{
    Const,
    Var,
};

class VarInfo
{
public:
    VarType type;
    std::string name;
    int32_t runtimeAddress;

    explicit VarInfo(VarType type, std::string name, int32_t runtimeAddress):
    type(type),
    name(std::move(name)),
    runtimeAddress(runtimeAddress)
    {
    }
};

class ProcedureInfo
{
public:
    ProcedureInfo* parent = nullptr;

    int32_t codeAddress = 0;
    int32_t level = 0;

    std::string name;
    std::vector<VarInfo> params;

    std::vector<VarInfo> vars;
    std::vector<ProcedureInfo> subProcedures;

    ProcedureInfo(int32_t codeAddress, int32_t level, std::string name):
    codeAddress(codeAddress),
    level(level),
    name(std::move(name))
    {
    }

    ProcedureInfo() = default;
};

class Parser
{
private:
    Tokenizer& tokenizer;
    std::optional<Token> token;
    int i = 0;

    std::vector<ILInst> code;
    int32_t level = 0;

    // ProcedureInfo program;

    std::vector<ProcedureInfo*> path;

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
            throw std::runtime_error("预期下一个Token, 但已经到达文件尾部");
        }
        return token.value();
    }

    const Token& Current(std::string expected)
    {
        if (!token.has_value())
        {
            throw std::runtime_error("预期" + expected + ", 但已经到达文件尾部");
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
        Token tkName = Match(TokenKind::Identifier);
        Match(TokenKind::Semi);

        auto program = ProcedureInfo(0, level++, tkName.GetString());
        path.push_back(&program);
        Block();
        path.pop_back();
    }

    void Block()
    {
        auto tk = Current("const, var, procedure或begin");
        if (tk.kind == TokenKind::Const)
        {
            Condecl();
        }
        if (tk.kind == TokenKind::Var)
        {
            Vardecl();
        }
        if (tk.kind == TokenKind::Procedure)
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
        ProcedureInfo& procedure = *path.back();

        auto tkId = Match(TokenKind::Identifier);
        Match(TokenKind::ColonEqual);
        auto tkInt = Match(TokenKind::Int);

        const auto it = std::ranges::find_if(procedure.vars, [&](auto& var) {
            return var.name == tkId.GetString();
        });
        if (it != procedure.vars.end())
        {
            throw std::runtime_error(GetErrorPrefix(tkId.fileLocation) + tkId.GetString() + "重复定义");
        }

        code.emplace_back(ILInstType::INT, 0, 1);
        code.emplace_back(ILInstType::LIT, 0, tkInt.GetInt32());
        code.emplace_back(ILInstType::STO, 0, procedure.vars.size());

        procedure.vars.emplace_back(VarType::Const, tkId.GetString(), procedure.vars.size());
    }

    void Vardecl()
    {
        ProcedureInfo& procedure = *path.back();

        Match(TokenKind::Var);
        auto tkId = Match(TokenKind::Identifier);

        if (std::ranges::find_if(procedure.vars, [&](auto& var) { return var.name == tkId.GetString();})
            != procedure.vars.end())
        {
            throw std::runtime_error(GetErrorPrefix(tkId.fileLocation) + tkId.GetString() + "重复定义");
        }

        code.emplace_back(ILInstType::INT, 0, 1);
        procedure.vars.emplace_back(VarType::Var, tkId.GetString(), procedure.vars.size());

        while (TryMatch(TokenKind::Comma))
        {
            tkId = Match(TokenKind::Identifier);

            if (std::ranges::find_if(procedure.vars, [&](auto& var) {return var.name == tkId.GetString();})
                != procedure.vars.end())
            {
                throw std::runtime_error(GetErrorPrefix(tkId.fileLocation) + tkId.GetString() + "重复定义");
            }

            code.emplace_back(ILInstType::INT, 0, 1);
            procedure.vars.emplace_back(VarType::Var, tkId.GetString(), procedure.vars.size());
        }
    }

    // Procedure
    void Proc()
    {
        ProcedureInfo& procedure = *path.back();

        Match(TokenKind::Procedure);
        auto tkName = Match(TokenKind::Identifier);

        if (tkName.GetString() == procedure.name ||
            std::ranges::find_if(procedure.subProcedures, [&](auto& subProc)
            {
                return subProc.name == tkName.GetString();
            }) != procedure.subProcedures.end())
        {
            throw std::runtime_error(GetErrorPrefix(tkName.fileLocation) + tkName.GetString() + "过程重复定义");
        }

        Match(TokenKind::LParen);

        ProcedureInfo subProcedure(code.size(), level++, tkName.GetString());

        std::vector<Token> tkParams;

        if (CurrentKindIs(TokenKind::Identifier))
        {
            tkParams.push_back(*token);
            while (TryMatch(TokenKind::Comma))
            {
                tkParams.push_back(Match(TokenKind::Identifier));
            }
        }

        for (int i = 0; i < tkParams.size(); i++)
        {
            // 最后一个参数位于-4
            subProcedure.params.emplace_back(VarType::Var, tkParams[i].GetString(), i - tkParams.size() - 3);
        }

        Match(TokenKind::RParen);
        Match(TokenKind::Semi);

        procedure.subProcedures.push_back(std::move(subProcedure));
        path.push_back(&procedure.subProcedures.back());

        Block();

        path.pop_back();
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
        ProcedureInfo& procedure = *path.back();

        switch (Current().kind)
        {
        case TokenKind::Identifier:
            {
                Token tkId = Match(TokenKind::Identifier);
                Match(TokenKind::ColonEqual);
                Exp();
                GenStore(tkId);
                break;
            }
        case TokenKind::If:
            {
                Match(TokenKind::If);
                Lexp();
                ILInst& jpc = code.emplace_back(ILInstType::JPC, 0, 0);
                Match(TokenKind::Then);
                Statement();
                if (TryMatch(TokenKind::Else))
                {
                    ILInst& jmp = code.emplace_back(ILInstType::JMP, 0, 0);
                    jpc.A = code.size();
                    Statement();
                    jmp.A = code.size();
                }
                else
                {
                    jpc.A = code.size();
                }
                break;
            }
        case TokenKind::While:
            {
                Match(TokenKind::While);
                int whileStart = code.size();
                Lexp();
                ILInst& jpc = code.emplace_back(ILInstType::JPC, 0, 0);
                Match(TokenKind::Do);
                Statement();
                code.emplace_back(ILInstType::JMP, 0, whileStart);
                jpc.A = code.size();
                break;
            }
            // call <id>([<exp>{,<exp>}])
        case TokenKind::Call:
            {
                // 过程允许调用自身、同级过程和直接子过程，在此处两者都被解析完了
                Match(TokenKind::Call);
                Token tkProcedureName = Match(TokenKind::Identifier);

                size_t paramsCount = 0;
                Match(TokenKind::LParen);
                if (!TryMatch(TokenKind::RParen))
                {
                    Exp();
                    paramsCount++;
                    while (TryMatch(TokenKind::Comma))
                    {
                        Exp();
                        paramsCount++;
                    }
                    Match(TokenKind::RParen);
                }

                if (procedure.name == tkProcedureName.GetString())
                {
                    // 调用自身
                    if (paramsCount != procedure.params.size())
                    {
                        throw std::runtime_error(GetErrorPrefix(tkProcedureName.fileLocation) +
                            tkProcedureName.GetString() + "过程参数个数不匹配");
                    }
                    code.emplace_back(ILInstType::CAL, 1, procedure.codeAddress);
                }
                else if (const auto it = std::ranges::find_if(
                    procedure.subProcedures,
                    [&](auto& subProcedure) {
                        return subProcedure.name == tkProcedureName.GetString();
                    });
                    it != procedure.subProcedures.end())
                {
                    // 调用直接子过程
                    if (paramsCount != it->params.size())
                    {
                        throw std::runtime_error(GetErrorPrefix(tkProcedureName.fileLocation) +
                            tkProcedureName.GetString() + "过程参数个数不匹配");
                    }
                    code.emplace_back(ILInstType::CAL, 0, it->codeAddress);
                }
                else
                {
                    // 调用同级过程没处理
                    throw std::runtime_error(GetErrorPrefix(tkProcedureName.fileLocation) +
                        tkProcedureName.GetString() + "过程未定义");
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
                auto tk = Match(TokenKind::Identifier);

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

    void GenLoad(const Token& tk)
    {
        int l = 0;
        for (auto procIt = path.rbegin(); procIt != path.rend(); ++procIt)
        {
            const auto it =
                std::ranges::find_if((*procIt)->vars, [tk](const VarInfo& var)
                {
                    return var.name == tk.GetString();
                });
            if (it != (*procIt)->vars.end())
            {
                code.emplace_back(ILInstType::LOD, l, it->runtimeAddress);
                return;
            }
            l++;
        }
        throw std::runtime_error(GetErrorPrefix(tk.fileLocation) + "未定义的标识符");
    }

    // <factor> -> <id>|<integer>|(<exp>)
    // factor.first = id, int, (
    void Factor()
    {
        ProcedureInfo& procedure = *path.back();

        std::optional<Token> tk;
        tk = TryMatch(TokenKind::Identifier);
        if (tk)
        {
            GenLoad(*tk);
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

    void GenStore(const Token& tk)
    {
        int l = 0;
        for (auto procIt = path.rbegin(); procIt != path.rend(); ++procIt)
        {
            const auto it =
                std::ranges::find_if((*procIt)->vars, [tk](const VarInfo& var)
                {
                    return var.name == tk.GetString();
                });
            if (it != (*procIt)->vars.end())
            {
                code.emplace_back(ILInstType::STO, l, it->runtimeAddress);
                return;
            }
            l++;
        }
        throw std::runtime_error(GetErrorPrefix(tk.fileLocation) + "未定义的标识符");
    }
};
