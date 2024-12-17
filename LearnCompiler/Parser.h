#pragma once
#include <iostream>

#include "ILInst.h"
#include "ILInstOprType.h"
#include "ProcedureInfo.h"
#include "Tokenizer.h"
#include "VarInfo.h"

class Parser
{
private:
    Tokenizer& tokenizer;
    std::optional<Token> token;

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

    [[noreturn]]
    static void ThrowMismatch(const Token& tk, const std::string& expected)
    {
        throw std::runtime_error(GetErrorMessage(tk, expected));
    }

    [[noreturn]]
    static void ThrowMismatch(const Token& tk, const TokenKind expected)
    {
        throw std::runtime_error(GetErrorPrefix(tk.fileLocation) +
            "预期" + std::string(magic_enum::enum_name(expected)) +
            "，但实际为" + tk.ToString());
    }

    [[noreturn]]
    static void ThrowMismatch(const Token& tk, const std::vector<TokenKind>& expected)
    {
        std::string message = GetErrorPrefix(tk.fileLocation) + "预期";
        for (auto it = expected.begin(); it != expected.end(); ++it)
        {
            message += magic_enum::enum_name(*it);
            if (it != expected.end() - 1)
            {
                message += "、";
            }
        }
        message += "，但实际为" + tk.ToString();
        throw std::runtime_error(message);
    }

    [[noreturn]]
    static void ThrowEOF(const TokenKind excepted)
    {
        std::string message = "预期" + std::string(magic_enum::enum_name(excepted)) + "，但已经到达文件尾部";
        throw std::runtime_error(message);
    }

    [[noreturn]]
    static void ThrowEOF(const std::vector<TokenKind>& excepted)
    {
        std::string message = "预期";
        for (auto it = excepted.begin(); it != excepted.end(); ++it)
        {
            message += magic_enum::enum_name(*it);
            if (it != excepted.end() - 1)
            {
                message += "、";
            }
        }
        message += "，但已经到达文件尾部";
        throw std::runtime_error(message);
    }

    std::string static GetErrorPrefix(const FileLocation location)
    {
        return "[语法错误] 位于" + location.ToString() + ": ";
    }

    std::string static GetErrorMessage(const Token& tk, const std::string& expected)
    {
        return GetErrorPrefix(tk.fileLocation) + "预期" + expected + "，但实际为" + tk.ToString();
    }

    Token Match(const TokenKind kind)
    {
        if (!token)
        {
            ThrowEOF(kind);
        }

        Token tk = token.value();
        if (tk.kind != kind)
        {
            ThrowMismatch(tk, kind);
        }

        token = tokenizer.GetToken();
        return tk;
    }

    Token Match(const std::vector<TokenKind>& kind)
    {
        if (!token)
        {
            ThrowEOF(kind);
        }

        Token tk = token.value();
        for (auto it = kind.begin(); it != kind.end(); ++it)
        {
            if (tk.kind == *it)
            {
                token = tokenizer.GetToken();
                return tk;
            }
        }
        ThrowMismatch(tk, kind);
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

    bool TryCurrent(const std::vector<TokenKind>& expected) const
    {
        return token && std::ranges::find(expected, token->kind) != expected.end();
    }

    Token Current()
    {
        if (!token.has_value())
        {
            throw std::runtime_error("预期下一个Token, 但已经到达文件尾部");
        }
        return token.value();
    }

    const Token& Current(const std::string& expected)
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

        auto program = ProcedureInfo(0, tkName.String());
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
        for (auto it = procedure->vars.begin(); it != procedure->vars.end() && it->attribute == VarAttribute::Const; ++it)
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
            code.emplace_back(ILInstType::INT, 0, -static_cast<int>(procedure->vars.size()));
        }
    }

    // <condecl> -> const <const>{,<const>};
    void Condecl()
    {
        Match(TokenKind::Const);
        do
        {
            Const();
        } while (TryMatch(TokenKind::Comma));
        Match(TokenKind::Semi);
    }

    // <const> -> <id> := <integer>
    void Const()
    {
        ProcedureInfo& procedure = *path.back();

        auto tkId = Match(TokenKind::Identifier);
        Match(TokenKind::ColonEqual);
        auto tkInt = Match(TokenKind::Int32);

        if (ProcedureContainsVar(procedure, tkId.String()))
        {
            throw std::runtime_error(GetErrorPrefix(tkId.fileLocation) + tkId.String() + "重复定义");
        }

        procedure.vars.emplace_back(VarAttribute::Const, VarType::I32, tkId.String(), procedure.vars.size());
        procedure.vars.back().value = tkInt.Int32();
    }

    // <vardecl> -> var <id>:<id>{,<id>:<id>};
    void Vardecl()
    {
        ProcedureInfo& procedure = *path.back();

        Match(TokenKind::Var);
        do
        {
            auto tkId = Match(TokenKind::Identifier);
            Match(TokenKind::Colon);
            Token tkType = Current("类型");

            VarInfo varInfo(VarAttribute::Var, VarType::I32, tkId.String(), procedure.vars.size());

            if (ProcedureContainsVar(procedure, tkId.String()))
            {
                throw std::runtime_error(GetErrorPrefix(tkId.fileLocation) + tkId.String() + "重复定义");
            }
            if (tkType.kind == TokenKind::I32)
            {
                varInfo.type = VarType::I32;
            }
            else if (tkType.kind == TokenKind::F32)
            {
                varInfo.type = VarType::F32;
            }
            else
            {
                ThrowMismatch(tkType, "i32或f32");
            }
            procedure.vars.emplace_back(varInfo);
        } while (TryMatch(TokenKind::Comma));
        Match(TokenKind::Semi);
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

    // procedure <id> ([<id>:<id>{,<id>:<id>}]) [: <id>] <block> [<proc>]
    void Proc()
    {
        // Proc name
        Match(TokenKind::Procedure);
        Token tkProcName = Match(TokenKind::Identifier);

        // Params
        std::vector<VarInfo> params;
        Match(TokenKind::LParen);
        if (!TryCurrent(TokenKind::RParen))
        {
            do
            {
                Token tkId = Match(TokenKind::Identifier);
                Match(TokenKind::Colon);
                Token tkType = Match({TokenKind::I32, TokenKind::F32});
                VarInfo varInfo(VarAttribute::Var, VarType::I32, tkId.String(), params.size());
                if (tkType.kind == TokenKind::F32)
                {
                    varInfo.type = VarType::F32;
                }
                params.emplace_back(varInfo);
            } while (TryMatch(TokenKind::Comma));
        }
        Match(TokenKind::RParen);

        // Ret
        std::optional<VarInfo> ret = std::nullopt;
        if (TryMatch(TokenKind::Colon))
        {
            Token tkRet = Match({TokenKind::I32, TokenKind::F32});
            ret = VarInfo(VarAttribute::Var, VarType::I32, "", -static_cast<int>(params.size()) - 4);
            if (tkRet.kind == TokenKind::F32)
            {
                ret->type = VarType::F32;
            }
        }

        // Gen
        ProcedureInfo& procedure = *path.back();

        ProcedureInfo subProcedure(code.size(),tkProcName.String());

        if (tkProcName.String() == procedure.name ||
            std::ranges::find_if(procedure.subProcedures, [&](auto& subProc)
            {
                return subProc.name == tkProcName.String();
            }) != procedure.subProcedures.end())
        {
            throw std::runtime_error(GetErrorPrefix(tkProcName.fileLocation) + tkProcName.String() + "过程重复定义");
        }

        for (int i = 0; i < params.size(); i++)
        {
            // 最后一个形参位于-4
            params[i].runtimeAddress = i - params.size() - 3;
        }
        subProcedure.params = std::move(params);

        subProcedure.ret = std::move(ret);

        procedure.subProcedures.push_back(std::move(subProcedure));
        path.push_back(&procedure.subProcedures.back());
        Block();
        code.emplace_back(ILInstType::OPR, 0, static_cast<int>(ILInstOprType::Ret));
        path.pop_back();

        if (TryCurrent(TokenKind::Procedure))
        {
            Proc();
        }
    }

    // <body> -> begin <statement>{<statement>} end
    void Body()
    {
        Match(TokenKind::Begin);
        do
        {
            Statement();
        } while (!TryMatch(TokenKind::End));
    }

    ProcedureInfo& GetProcedure(const Token& tk, int* pL)
    {
        int l = 0;
        // 逆序遍历匹配过程path上的所有过程的子过程，所以program自己不会被匹配
        for (auto it = path.rbegin(); it != path.rend(); ++it)
        {
            ProcedureInfo* procedure = *it;
            if (const auto it1 = std::ranges::find_if(
                    procedure->subProcedures,
                    [&](auto& subProcedure)
                    {
                        return subProcedure.name == tk.String();
                    });
                it1 != procedure->subProcedures.end())
            {
                *pL = l;
                return *it1;
            }
            l++;
        }
        throw std::runtime_error(GetErrorPrefix(tk.fileLocation) + tk.String() + "过程未定义");
    }

    // <CallProcedure> -> <id> ([<exp>{,<exp>}])
    // 此处已经匹配完(
    std::optional<VarType> CallProcedure(const Token& tkId, bool needRet)
    {
        int l;
        ProcedureInfo& target = GetProcedure(tkId, &l);
        int reserveForRet;
        if (target.ret)
        {
            reserveForRet = 1;
            code.emplace_back(ILInstType::INT, 0, 1);
        }
        else
        {
            reserveForRet = 0;
            if (needRet)
            {
                throw std::runtime_error(GetErrorPrefix(tkId.fileLocation) + tkId.String() + "过程需要返回值");
            }
        }

        size_t paramsCount = 0;
        if (!TryMatch(TokenKind::RParen))
        {
            do
            {
                Exp();
                paramsCount++;
            } while (TryMatch(TokenKind::Comma));
            Match(TokenKind::RParen);
        }

        if (paramsCount != target.params.size())
        {
            throw std::runtime_error(GetErrorPrefix(tkId.fileLocation) + tkId.String() +
                "过程参数个数不匹配");
        }
        code.emplace_back(ILInstType::CAL, l, target.codeAddress);

        if (needRet)
        {
            if (paramsCount != 0)
            {
                code.emplace_back(ILInstType::INT, 0, -static_cast<int>(paramsCount));
            }
        }
        else
        {
            if ((reserveForRet + paramsCount) != 0)
            {
                code.emplace_back(ILInstType::INT, 0, -static_cast<int>(reserveForRet + paramsCount));
            }
        }

        return target.ret ? target.ret->type : std::nullopt;
    }

/*
<statement> ->  <id> := <exp>;
               |<CallProcedure>;
               |if <lexp> then <statement>[else <statement>]
               |while <lexp> do <statement>
               |<body>
               |read (<id>{，<id>});
               |write (<exp>{, <exp>});
               |return [<exp>];
*/
    void Statement()
    {
        ProcedureInfo& procedure = *path.back();

        Token tk = Current("statement");
        if (tk.kind == TokenKind::Identifier)
        {
            MoveNext();
            if (TryMatch(TokenKind::ColonEqual))
            {
                int l;
                auto varInfo = FindVar(tk, &l);
                Exp();
                code.emplace_back(ILInstType::STO, l, varInfo.runtimeAddress);
                Match(TokenKind::Semi);
            }
            else if (TryMatch(TokenKind::LParen))
            {
                CallProcedure(tk, false);
                Match(TokenKind::Semi);
            }
            else
            {
                throw std::runtime_error(GetErrorPrefix(tk.fileLocation) + "必须为:=或(");
            }
        }
        else if (tk.kind == TokenKind::If)
        {
            MoveNext();
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
        }
        else if (tk.kind == TokenKind::While)
        {
            MoveNext();
            int whileStart = code.size();
            Lexp();
            code.emplace_back(ILInstType::JPC, 0, 0);
            int jpcIdx = code.size() - 1;
            Match(TokenKind::Do);
            Statement();
            code.emplace_back(ILInstType::JMP, 0, whileStart);
            code[jpcIdx].A = code.size();
        }
        else if (tk.kind == TokenKind::Begin)
        {
            Body();
        }
        else if (tk.kind == TokenKind::Read)
        {
            MoveNext();
            Match(TokenKind::LParen);
            do
            {
                auto tkVar = Match(TokenKind::Identifier);
                int l;
                auto varInfo = FindVar(tkVar, &l);
                code.emplace_back(ILInstType::RED, l, varInfo.runtimeAddress);
            } while (TryMatch(TokenKind::Comma));
            Match(TokenKind::RParen);
            Match(TokenKind::Semi);
        }
        else if (tk.kind == TokenKind::Write)
        {
            MoveNext();
            Match(TokenKind::LParen);
            Exp();
            code.emplace_back(ILInstType::WRT, 0, 0);
            while (TryMatch(TokenKind::Comma))
            {
                Exp();
                code.emplace_back(ILInstType::WRT, 0, 0);
            }
            Match(TokenKind::RParen);
            Match(TokenKind::Semi);
        }
        else if (tk.kind == TokenKind::Return)
        {
            MoveNext();
            if (TryMatch(TokenKind::Semi))
            {
                code.emplace_back(ILInstType::OPR, 0, static_cast<int32_t>(ILInstOprType::Ret));
            }
            else
            {
                if (!procedure.ret)
                {
                    throw std::runtime_error(GetErrorPrefix(tk.fileLocation) + "过程没有返回值");
                }
                Exp();
                Match(TokenKind::Semi);
                code.emplace_back(ILInstType::STO, 0, procedure.ret->runtimeAddress);
                code.emplace_back(ILInstType::OPR, 0, static_cast<int32_t>(ILInstOprType::Ret));
            }
        }
        else
        {
            throw std::runtime_error(GetErrorPrefix(tk.fileLocation) + "Statement错误");
        }

    }

    // <lexp> → <exp> <lop> <exp>|odd <exp>
    // Logical Expression
    void Lexp()
    {
        if (auto tk = TryMatch(TokenKind::Odd))
        {
            auto t = Exp();
            if (t != VarType::I32)
            {
                throw std::runtime_error(GetErrorPrefix(tk->fileLocation) + "odd运算符只能用于整型");
            }
            code.emplace_back(ILInstType::OPR, 0, static_cast<int32_t>(ILInstOprType::Odd));
        }
        else
        {
            auto t1 = Exp();
            const auto tkLop = Lop();
            auto t2 = Exp();
            code.push_back(GenLop(tkLop, t1, t2));
        }
    }

    // exp.first = +, -, id, int, (
    // <exp> -> [+|-]<term>{<aop><term>}
    // Expression
    VarType Exp()
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
        auto t1 = Term();
        if (flag)
        {
            if (t1 == VarType::I32)
            {
                code.emplace_back(ILInstType::OPR, 0, static_cast<int32_t>(ILInstOprType::Neg));
            }
            else if (t1 == VarType::F32)
            {
                code.emplace_back(ILInstType::OPR, 0, static_cast<int32_t>(ILInstOprType::FNeg));
            }
        }
        while (TryCurrent({TokenKind::Plus, TokenKind::Minus}))
        {
            auto tkAop = Aop();
            auto t2 = Term();
            GenAop(tkAop, t1, t2);
        }
        return t1;
    }

    // <term> -> <factor>{<mop><factor>}
    // term.first = id, int, (
    VarType Term()
    {
        auto t1 = Factor();
        while (TryCurrent({TokenKind::Star, TokenKind::Slash}))
        {
            auto tkMop = Mop();
            auto t2 = Factor();
            if (t1 != t2)
            {
                throw std::runtime_error(GetErrorPrefix(tkMop.fileLocation) + "左右操作数类型不匹配");
            }
            GenMop(tkMop, t1);
        }
        return t1;
    }

    // <factor> ->  <id>
    //             |<CallProcedure>
    //             |<integer>
    //             |(<exp>)
    // factor.first = id, int, (
    VarType Factor()
    {
        const auto tk = Current("Factor");

        if (tk.kind == TokenKind::Identifier)
        {
            MoveNext();
            if (TryMatch(TokenKind::LParen))
            {
                auto t = CallProcedure(tk, true);
                return *t;
            }
            int l;
            auto varInfo = FindVar(tk, &l);
            code.emplace_back(ILInstType::LOD, l, varInfo.runtimeAddress);
            return varInfo.type;
        }
        if (tk.kind == TokenKind::Int32)
        {
            MoveNext();
            code.emplace_back(ILInstType::LIT, 0, tk.Int32());
            return VarType::I32;
        }
        if (tk.kind == TokenKind::Float32)
        {
            MoveNext();
            float f = tk.Float32();
            int i = *reinterpret_cast<int*>(&f);
            code.emplace_back(ILInstType::LIT, 0, i);
            return VarType::F32;
        }
        if (tk.kind == TokenKind::LParen)
        {
            auto type = Exp();
            Match(TokenKind::RParen);
            return type;
        }
        if (tk.kind == TokenKind::I32)
        {
            MoveNext();
            Match(TokenKind::LParen);
            auto type = Exp();
            Match(TokenKind::RParen);
            if (type == VarType::F32)
            {
                code.emplace_back(ILInstType::OPR, 0, static_cast<int32_t>(ILInstOprType::FToI));
            }
            return VarType::I32;
        }
        if (tk.kind == TokenKind::F32)
        {
            MoveNext();
            Match(TokenKind::LParen);
            auto type = Exp();
            Match(TokenKind::RParen);
            if (type == VarType::I32)
            {
                code.emplace_back(ILInstType::OPR, 0, static_cast<int32_t>(ILInstOprType::IToF));
            }
            return VarType::F32;
        }
        ThrowMismatch(tk, "Factor");
    }

    // <lop> -> =|<>|<|<=|>|>=
    Token Lop()
    {
        const auto tk = Match({
            TokenKind::Equal, TokenKind::LessGreater,
            TokenKind::Less, TokenKind::LessEqual,
            TokenKind::Greater, TokenKind::GreaterEqual});
        return tk;
    }

    ILInst GenLop(const Token& tkLop, VarType t1, VarType t2)
    {
        if (t1 != t2)
        {
            throw std::runtime_error(GetErrorPrefix(tkLop.fileLocation) + "左右操作数类型不匹配");
        }

        if (t1 == VarType::I32)
        {
            switch (tkLop.kind)
            {
            case TokenKind::Equal:
                {
                    return {ILInstType::OPR, 0, static_cast<int32_t>(ILInstOprType::Eql)};
                }
            case TokenKind::LessGreater:
                {
                    return {ILInstType::OPR, 0, static_cast<int32_t>(ILInstOprType::Neq)};
                }
            case TokenKind::Less:
                {
                    return {ILInstType::OPR, 0, static_cast<int32_t>(ILInstOprType::Lss)};
                }
            case TokenKind::LessEqual:
                {
                    return {ILInstType::OPR, 0, static_cast<int32_t>(ILInstOprType::Leq)};
                }
            case TokenKind::Greater:
                {
                    return {ILInstType::OPR, 0, static_cast<int32_t>(ILInstOprType::Gtr)};
                }
            case TokenKind::GreaterEqual:
                {
                    return {ILInstType::OPR, 0, static_cast<int32_t>(ILInstOprType::Geq)};
                }
            }
        }
        else if (t1 == VarType::F32)
        {
            switch (tkLop.kind)
            {
            case TokenKind::Equal:
                {
                    return {ILInstType::OPR, 0, static_cast<int32_t>(ILInstOprType::FEql)};
                }
            case TokenKind::LessGreater:
                {
                    return {ILInstType::OPR, 0, static_cast<int32_t>(ILInstOprType::FNeq)};
                }
            case TokenKind::Less:
                {
                    return {ILInstType::OPR, 0, static_cast<int32_t>(ILInstOprType::FLss)};
                }
            case TokenKind::LessEqual:
                {
                    return {ILInstType::OPR, 0, static_cast<int32_t>(ILInstOprType::FLeq)};
                }
            case TokenKind::Greater:
                {
                    return {ILInstType::OPR, 0, static_cast<int32_t>(ILInstOprType::FGtr)};
                }
            case TokenKind::GreaterEqual:
                {
                    return {ILInstType::OPR, 0, static_cast<int32_t>(ILInstOprType::FGeq)};
                }
            }
        }
        throw std::runtime_error(GetErrorPrefix(tkLop.fileLocation) + "逻辑运算符不匹配");
    }

    // <aop> -> +|-
    Token Aop()
    {
        const auto tk = Match({TokenKind::Plus, TokenKind::Minus});
        return tk;
    }

    ILInst GenAop(const Token& tkAop, VarType t1, VarType t2)
    {
        if (t1 != t2)
        {
            throw std::runtime_error(GetErrorPrefix(tkAop.fileLocation) + "左右操作数类型不匹配");
        }

        if (tkAop.kind == TokenKind::Plus)
        {
            if (t1 == VarType::I32)
            {
                return{ILInstType::OPR, 0, static_cast<int32_t>(ILInstOprType::Add)};
            }
            if (t1 == VarType::F32)
            {
                return{ILInstType::OPR, 0, static_cast<int32_t>(ILInstOprType::FAdd)};
            }
        }
        if (tkAop.kind == TokenKind::Minus)
        {
            if (t1 == VarType::I32)
            {
                return{ILInstType::OPR, 0, static_cast<int32_t>(ILInstOprType::Sub)};
            }
            if (t1 == VarType::F32)
            {
                return{ILInstType::OPR, 0, static_cast<int32_t>(ILInstOprType::FSub)};
            }
        }
        throw std::runtime_error(GetErrorPrefix(tkAop.fileLocation) + "算术运算符不匹配");
    }

    // <mop> -> *|/
    Token Mop()
    {
        const auto tk = Match({TokenKind::Star, TokenKind::Slash});
        return tk;
    }

    void GenMop(const Token& tkMop, VarType type)
    {
        if (tkMop.kind == TokenKind::Star)
        {
            if (type == VarType::I32)
            {
                code.emplace_back(ILInstType::OPR, 0, static_cast<int32_t>(ILInstOprType::Mul));
                return;
            }
            if (type == VarType::F32)
            {
                code.emplace_back(ILInstType::OPR, 0, static_cast<int32_t>(ILInstOprType::FMul));
                return;
            }
        }
        if (tkMop.kind == TokenKind::Slash)
        {
            if (type == VarType::I32)
            {
                code.emplace_back(ILInstType::OPR, 0, static_cast<int32_t>(ILInstOprType::Div));
                return;
            }
            if (type == VarType::F32)
            {
                code.emplace_back(ILInstType::OPR, 0, static_cast<int32_t>(ILInstOprType::FDiv));
                return;
            }
        }
        ThrowMismatch(tkMop, "Mop");
    }

    VarInfo FindVar(const Token& tkVar, int* pL)
    {
        int l = 0;
        const std::string& name = tkVar.String();
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
        throw std::runtime_error(GetErrorPrefix(tkVar.fileLocation) + tkVar.String() + "未定义");
    }
};
