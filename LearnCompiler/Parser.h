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
    int32_t value = 0;

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

    int32_t level = 0;

    std::vector<ProcedureInfo*> path;

public:
    std::vector<ILInst> code;

    explicit Parser(Tokenizer& tokenizer):
    tokenizer(tokenizer)
    {
    }

    void Parse()
    {
        token = tokenizer.GetToken();
        Prog();
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
        if (!token || token->kind != kind)
        {
            return std::nullopt;
        }

        auto tk = *token;

        token = tokenizer.GetToken();
        return tk;
    }

    bool TryCurrent(const TokenKind kind) const
    {
        return token && token->kind == kind;
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

        auto program = ProcedureInfo(0, level++, tkName.String());
        path.push_back(&program);
        Block();
        path.pop_back();
    }

    void Block()
    {
        ProcedureInfo* procedure = path.back();

        if (TryCurrent(TokenKind::Const))
        {
            Condecl();
        }
        if (TryCurrent(TokenKind::Var))
        {
            Vardecl();
        }

        if (!procedure->vars.empty())
        {
            code.emplace_back(ILInstType::INT, 0, procedure->vars.size());
        }
        for (auto it = procedure->vars.begin(); it != procedure->vars.end() && it->type == VarType::Const; ++it)
        {
            code.emplace_back(ILInstType::LIT, 0, it->value);
            code.emplace_back(ILInstType::STO, 0, it->runtimeAddress);
        }

        if (TryCurrent(TokenKind::Procedure))
        {
            code.emplace_back(ILInstType::JMP, 0, 0);
            int jmpIdx = static_cast<int>(code.size()) - 1;
            Proc();
            code[jmpIdx].A = static_cast<int>(code.size());
        }
        Body();
        if (!procedure->vars.empty())
        {
            code.emplace_back(ILInstType::INT, 0, -static_cast<int>(path.back()->vars.size()));
        }
    }

    void Condecl()
    {
        Match(TokenKind::Const);
        do
        {
            Const();
        } while (TryMatch(TokenKind::Comma));
    }

    void Const()
    {
        ProcedureInfo& procedure = *path.back();

        auto tkId = Match(TokenKind::Identifier);
        Match(TokenKind::ColonEqual);
        auto tkInt = Match(TokenKind::Int);

        if (ProcedureContainsVar(procedure, tkId.String()))
        {
            throw std::runtime_error(GetErrorPrefix(tkId.fileLocation) + tkId.String() + "重复定义");
        }

        procedure.vars.emplace_back(VarType::Const, tkId.String(), procedure.vars.size());
        procedure.vars.back().value = tkInt.Int32();
    }

    void Vardecl()
    {
        ProcedureInfo& procedure = *path.back();

        Match(TokenKind::Var);
        do
        {
            auto tkId = Match(TokenKind::Identifier);

            if (ProcedureContainsVar(procedure, tkId.String()))
            {
                throw std::runtime_error(GetErrorPrefix(tkId.fileLocation) + tkId.String() + "重复定义");
            }
            procedure.vars.emplace_back(VarType::Var, tkId.String(), procedure.vars.size());
        } while (TryMatch(TokenKind::Comma));
    }

    bool static ProcedureContainsVar(const ProcedureInfo& procedure, const std::string& name)
    {
        if (std::ranges::find_if(procedure.params, [&](auto& var) { return var.name == name; })
            != procedure.params.end())
        {
            return true;
        }
        if (std::ranges::find_if(procedure.vars, [&](auto& var) { return var.name == name; })
            != procedure.vars.end())
        {
            return true;
        }
        return false;
    }

    // Procedure
    void Proc()
    {
        ProcedureInfo& procedure = *path.back();

        Match(TokenKind::Procedure);
        auto tkName = Match(TokenKind::Identifier);

        if (tkName.String() == procedure.name ||
            std::ranges::find_if(procedure.subProcedures, [&](auto& subProc)
            {
                return subProc.name == tkName.String();
            }) != procedure.subProcedures.end())
        {
            throw std::runtime_error(GetErrorPrefix(tkName.fileLocation) + tkName.String() + "过程重复定义");
        }

        Match(TokenKind::LParen);

        ProcedureInfo subProcedure(code.size(), level++, tkName.String());

        std::vector<Token> tkParams;

        if (auto tkParam = TryMatch(TokenKind::Identifier))
        {
            tkParams.push_back(*tkParam);
            while (TryMatch(TokenKind::Comma))
            {
                tkParams.push_back(Match(TokenKind::Identifier));
            }
        }

        for (int i = 0; i < tkParams.size(); i++)
        {
            // 最后一个参数位于-4
            subProcedure.params.emplace_back(VarType::Var, tkParams[i].String(), i - tkParams.size() - 3);
        }

        Match(TokenKind::RParen);
        Match(TokenKind::Semi);

        procedure.subProcedures.push_back(std::move(subProcedure));
        path.push_back(&procedure.subProcedures.back());
        Block();
        code.emplace_back(ILInstType::OPR, 0, static_cast<int>(ILInstOprType::Ret));
        path.pop_back();

        if (TryMatch(TokenKind::Semi))
        {
            Proc();
        }
    }

    // <body> -> begin <statement>{;<statement>}end
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
                int l;
                auto varInfo = FindVar(tkId.String(), &l);
                if (!varInfo)
                {
                    throw std::runtime_error(GetErrorPrefix(tkId.fileLocation) + tkId.String() + "找不到");
                }
                Match(TokenKind::ColonEqual);
                Exp();
                code.emplace_back(ILInstType::STO, l, varInfo->runtimeAddress);
                break;
            }
        case TokenKind::If:
            {
                Match(TokenKind::If);
                Lexp();
                code.emplace_back(ILInstType::JPC, 0, 0);
                int jpcIdx = code.size() - 1;
                Match(TokenKind::Then);
                Statement();
                if (TryMatch(TokenKind::Else))
                {
                    code.emplace_back(ILInstType::JMP, 0, 0);
                    int jmpIdx = code.size() - 1;
                    code[jpcIdx].A = code.size();
                    Statement();
                    code[jmpIdx].A = code.size();
                }
                else
                {
                    code[jpcIdx].A = code.size();
                }
                break;
            }
        case TokenKind::While:
            {
                Match(TokenKind::While);
                int whileStart = code.size();
                Lexp();
                code.emplace_back(ILInstType::JPC, 0, 0);
                int jpcIdx = code.size() - 1;
                Match(TokenKind::Do);
                Statement();
                code.emplace_back(ILInstType::JMP, 0, whileStart);
                code[jpcIdx].A = code.size();
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

                if (procedure.name == tkProcedureName.String())
                {
                    // 调用自身
                    if (paramsCount != procedure.params.size())
                    {
                        throw std::runtime_error(GetErrorPrefix(tkProcedureName.fileLocation) +
                            tkProcedureName.String() + "过程参数个数不匹配");
                    }
                    code.emplace_back(ILInstType::CAL, 1, procedure.codeAddress);
                    code.emplace_back(ILInstType::INT, 0, -(int)paramsCount);
                }
                else if (const auto it = std::ranges::find_if(
                    procedure.subProcedures,
                    [&](auto& subProcedure) {
                        return subProcedure.name == tkProcedureName.String();
                    });
                    it != procedure.subProcedures.end())
                {
                    // 调用直接子过程
                    if (paramsCount != it->params.size())
                    {
                        throw std::runtime_error(GetErrorPrefix(tkProcedureName.fileLocation) +
                            tkProcedureName.String() + "过程参数个数不匹配");
                    }
                    code.emplace_back(ILInstType::CAL, 0, it->codeAddress);
                    code.emplace_back(ILInstType::INT, 0, -(int)paramsCount);
                }
                else
                {
                    // 调用同级过程没处理
                    throw std::runtime_error(GetErrorPrefix(tkProcedureName.fileLocation) +
                        tkProcedureName.String() + "过程未定义");
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
                do
                {
                    auto tk = Match(TokenKind::Identifier);

                    int l;
                    if (auto varInfo = FindVar(tk.String(), &l))
                    {
                        code.emplace_back(ILInstType::RED, l, varInfo->runtimeAddress);
                    }
                    else
                    {
                        throw std::runtime_error(GetErrorPrefix(tk.fileLocation) + tk.String() + "未定义");
                    }
                } while (TryMatch(TokenKind::Comma));

                Match(TokenKind::RParen);
                break;
            }
        case TokenKind::Write:
            {
                Match(TokenKind::Write);
                Match(TokenKind::LParen);
                Exp();
                code.emplace_back(ILInstType::WRT, 0, 0);
                while (TryMatch(TokenKind::Comma))
                {
                    Exp();
                    code.emplace_back(ILInstType::WRT, 0, 0);
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
            auto inst = Aop();
            Term();
            code.push_back(inst);
        }
    }

    // <term> -> <factor>{<mop><factor>}
    // term.first = id, int, (
    void Term()
    {
        Factor();
        while (token && (token->kind == TokenKind::Star || token->kind == TokenKind::Slash))
        {
            auto inst = Mop();
            Factor();
            code.push_back(inst);
        }
    }

    // <factor> -> <id>|<integer>|(<exp>)
    // factor.first = id, int, (
    void Factor()
    {
        ProcedureInfo& procedure = *path.back();

        if (auto tk = TryMatch(TokenKind::Identifier))
        {
            int l;
            if (auto varInfo = FindVar(tk->String(), &l))
            {
                code.emplace_back(ILInstType::LOD, l, varInfo->runtimeAddress);
            }
            else
            {
                throw std::runtime_error(GetErrorPrefix(tk->fileLocation) + tk->String() + "未定义");
            }
        }
        else if (tk = TryMatch(TokenKind::Int);tk)
        {
            code.emplace_back(ILInstType::LIT, 0, tk->Int32());
        }
        else if (TryMatch(TokenKind::LParen))
        {
            Exp();
            Match(TokenKind::RParen);
        }
        else
        {
            throw std::runtime_error(GetErrorPrefix(Current().fileLocation) + "Factor 错误");
        }
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

    // <aop> -> +|-
    ILInst Aop()
    {
        const auto tk = Current("+或-");
        if (tk.kind == TokenKind::Plus)
        {
            MoveNext();
            return {ILInstType::OPR, 0, static_cast<int32_t>(ILInstOprType::Add)};
        }
        if (tk.kind == TokenKind::Minus)
        {
            MoveNext();
            return {ILInstType::OPR, 0, static_cast<int32_t>(ILInstOprType::Sub)};
        }
        throw std::runtime_error(GetErrorPrefix(tk.fileLocation) + "Aop 错误");
    }

    // <mop> -> *|/
    ILInst Mop()
    {
        const auto tk = Current("*或/");
        if (tk.kind == TokenKind::Star)
        {
            MoveNext();
            return {ILInstType::OPR, 0, static_cast<int32_t>(ILInstOprType::Mul)};
        }
        if (tk.kind == TokenKind::Slash)
        {
            MoveNext();
            return {ILInstType::OPR, 0, static_cast<int32_t>(ILInstOprType::Div)};
        }
        throw std::runtime_error(GetErrorPrefix(tk.fileLocation) + "Mop 错误");
    }

    std::optional<VarInfo> FindVar(const std::string& name, int* pL)
    {
        int l = 0;
        for (auto procIt = path.rbegin(); procIt != path.rend(); ++procIt)
        {
            const auto& vars = (*procIt)->vars;
            const auto& params = (*procIt)->params;

            auto it = std::ranges::find_if(vars, [name](const VarInfo& var)
            {
                return var.name == name;
            });
            if (it != vars.end())
            {
                *pL = l;
                return *it;
            }

            it = std::ranges::find_if(params, [name](const VarInfo& var)
            {
                return var.name == name;
            });
            if (it != params.end())
            {
                *pL = l;
                return *it;
            }

            l++;
        }
        return std::nullopt;
    }
};
