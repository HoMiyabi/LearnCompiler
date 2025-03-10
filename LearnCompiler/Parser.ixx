﻿module;
#include <iostream>
#include <vector>
#include <optional>
#include <variant>

#include "PtrHelper.h"

export module Parser;
import CallProcedureNode;
import ASTNode;
import BinaryNode;
import UnaryNode;
import UnaryNodeType;
import ValueNode;
import VarNode;
import BinaryNodeType;
import Token;
import Tokenizer;
import ProcedureInfo;
import ILInst;
import TokenKind;
import VarInfo;
import ILInstType;
import FileLocation;
import ILInstOprType;
import ErrorUtils;

export class Parser
{
private:
    // 参考类型的Tokenizer对象，用于文本分割和处理
    Tokenizer& tokenizer;
    
    // 可能存在的当前处理中的Token对象，用于词法分析
    std::optional<Token> token;
    
    // 保存过程信息指针的向量，表示当前解析路径上的过程信息
    std::vector<ProcedureInfo*> path;
public:
    std::vector<ILInst> code;

// 显式构造函数，初始化解析器
// 参数 tokenizer: 一个引用，指向词法分析器对象，用于获取和解析标记
explicit Parser(Tokenizer& tokenizer):
    tokenizer(tokenizer)
    {
    }

// 主要解析函数，负责调用其他函数以解析源代码
void Parse()
{
    // 从词法分析器获取第一个标记，开始解析过程
    token = tokenizer.GetToken();
    Prog();
}

private:
    [[noreturn]]
    static void ThrowMismatch(const Token& tk, const TokenKind expected)
    {
        ThrowSyntax(tk.filePath, tk.fileLocation,
            "预期" + std::string(to_string(expected)) +
            "，但实际为" + tk.ToString());
    }

    [[noreturn]]
    static void ThrowMismatch(const Token& tk, const std::vector<TokenKind>& expected)
    {
        std::string message = "预期";
        for (auto it = expected.begin(); it != expected.end(); ++it)
        {
            message += to_string(*it);
            if (it != expected.end() - 1)
            {
                message += "、";
            }
        }
        message += "，但实际为" + tk.ToString();
        ThrowSyntax(tk.filePath, tk.fileLocation, message);
    }

    [[noreturn]]
    void ThrowEOF(const TokenKind excepted) const
    {
        std::string message = "预期" + std::string(to_string(excepted)) + "，但已经到达文件尾部";
        ThrowSyntax(tokenizer.filePath, tokenizer.fileLocation, message);
    }

    [[noreturn]]
    void ThrowEOF(const std::vector<TokenKind>& excepted) const
    {
        std::string message = "预期";
        for (auto it = excepted.begin(); it != excepted.end(); ++it)
        {
            message += to_string(*it);
            if (it != excepted.end() - 1)
            {
                message += "、";
            }
        }
        message += "，但已经到达文件尾部";
        ThrowSyntax(tokenizer.filePath, tokenizer.fileLocation, message);
    }

    // std::string static GetErrorPrefix(const FileLocation location)
    // {
    //     return "[语法错误] 位于" + location.ToString() + ": ";
    // }

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

        Token tk = *token;

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

    Token Current(const std::vector<TokenKind>& excepted) const
    {
        if (!token)
        {
            ThrowEOF(excepted);
        }

        for (auto it = excepted.begin(); it != excepted.end(); ++it)
        {
            if (token->kind == *it)
            {
                return *token;
            }
        }
        ThrowMismatch(*token, excepted);
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

        auto program = ProcedureInfo(0, tkName.rawText);
        path.push_back(&program);
        Block();
        path.pop_back();
    }

    // <block> -> [<condecl>][<vardecl>][<proc>]<body>
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

        GenPushVars();
        for (auto it = procedure->vars.begin(); it != procedure->vars.end() && it->bConst; ++it)
        {
            if (it->type == VarType::I32)
            {
                code.emplace_back(ILInst::LIT(std::get<int32_t>(it->value)));
            }
            else if (it->type == VarType::F32)
            {
                code.emplace_back(ILInst::LIT(std::get<float>(it->value)));
            }
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
        GenPopVars();
    }

    void GenPushVars()
    {
        ProcedureInfo* procedure = path.back();
        if (!procedure->vars.empty())
        {
            code.emplace_back(ILInstType::INT, 0, procedure->vars.size());
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

    // <const> -> <const> -> <id> := <literal>
    void Const()
    {
        auto tkId = Match(TokenKind::Identifier);
        Match(TokenKind::ColonEqual);
        auto tkLiteral = Match({TokenKind::Int32, TokenKind::Float32});

        GenConst(tkId, tkLiteral);
    }

    void GenConst(const Token& tkId, const Token& tkLiteral) const
    {
        ProcedureInfo& procedure = *path.back();
        if (ProcedureContainsVar(procedure, tkId.rawText))
        {
            ThrowSemantic(tkId.filePath, tkId.fileLocation, tkId.rawText + "重复定义");
        }

        VarInfo varInfo(true, VarType::I32, tkId.rawText, procedure.vars.size());
        if (tkLiteral.kind == TokenKind::Int32)
        {
            varInfo.type = VarType::I32;
            varInfo.value = tkLiteral.Int32();
        }
        else if (tkLiteral.kind == TokenKind::Float32)
        {
            varInfo.type = VarType::F32;
            varInfo.value = tkLiteral.Float32();
        }

        procedure.vars.push_back(varInfo);
    }

    // <vardecl> -> var <id>:<type>{,<id>:<type>};
    void Vardecl()
    {
        Match(TokenKind::Var);
        do
        {
            auto tkId = Match(TokenKind::Identifier);
            Match(TokenKind::Colon);
            Token tkType = Match({TokenKind::I32, TokenKind::F32});
            GenOneVardecl(tkId, tkType);
        } while (TryMatch(TokenKind::Comma));
        Match(TokenKind::Semi);
    }

    void GenOneVardecl(const Token& tkId, const Token& tkType) const
    {
        ProcedureInfo& procedure = *path.back();
        if (ProcedureContainsVar(procedure, tkId.rawText))
        {
            ThrowSemantic(tkId.filePath, tkId.fileLocation, tkId.rawText + "重复定义");
        }
        VarInfo varInfo(false, VarType::I32, tkId.rawText, procedure.vars.size());
        if (tkType.kind == TokenKind::I32)
        {
            varInfo.type = VarType::I32;
        }
        else if (tkType.kind == TokenKind::F32)
        {
            varInfo.type = VarType::F32;
        }
        procedure.vars.push_back(varInfo);
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
                VarInfo varInfo(false, VarType::I32, tkId.rawText, params.size());
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
            ret = VarInfo(false, VarType::I32, "", -static_cast<int>(params.size()) - 4);
            if (tkRet.kind == TokenKind::F32)
            {
                ret->type = VarType::F32;
            }
        }

        // Gen
        ProcedureInfo& procedure = *path.back();

        ProcedureInfo subProcedure(code.size(),tkProcName.rawText);

        if (tkProcName.rawText == procedure.name ||
            std::ranges::find_if(procedure.subProcedures, [&](auto& subProc)
            {
                return subProc.name == tkProcName.rawText;
            }) != procedure.subProcedures.end())
        {
            ThrowSemantic(tkProcName.filePath, tkProcName.fileLocation, tkProcName.rawText + "过程重复定义");
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
                        return subProcedure.name == tk.rawText;
                    });
                it1 != procedure->subProcedures.end())
            {
                *pL = l;
                return *it1;
            }
            l++;
        }
        ThrowSemantic(tk.filePath, tk.fileLocation, tk.rawText + "过程未定义");
    }

    CallProcedureNode* CallProcedure(const Token& tkId, bool needRet)
    {
        int l;
        ProcedureInfo& proc = GetProcedure(tkId, &l);

        std::vector<ASTNode*> args;
        if (!TryMatch(TokenKind::RParen))
        {
            do
            {
                args.push_back(Exp());
            } while (TryMatch(TokenKind::Comma));
            Match(TokenKind::RParen);
        }
        return new CallProcedureNode(&proc, l, std::move(args), needRet, tkId);
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

        Token tk = Current({TokenKind::Identifier,
            TokenKind::If,
            TokenKind::While,
            TokenKind::Begin,
            TokenKind::Read,
            TokenKind::Write,
            TokenKind::Return});

        if (tk.kind == TokenKind::Identifier)
        {
            MoveNext();
            auto tk1 = Match({TokenKind::ColonEqual, TokenKind::LParen});
            if (tk1.kind == TokenKind::ColonEqual)
            {
                ASTNode* node(Exp());
                Match(TokenKind::Semi);

                node = node->CalTypeAndOptimize();
                int l;
                const VarInfo& varInfo = GetVar(tk, &l);
                if (node->varType != varInfo.type)
                {
                    ThrowSemantic(tk.filePath, tk.fileLocation, std::string("类型不匹配，将类型") +
                        to_string(node->varType) + "赋值给类型" + to_string(varInfo.type));
                }
                node->GenerateCode(code);

                code.emplace_back(ILInstType::STO, l, varInfo.runtimeAddress);
            }
            else if (tk1.kind == TokenKind::LParen)
            {
                ASTNode* node = std::move(CallProcedure(tk, false));
                Match(TokenKind::Semi);

                node = node->CalTypeAndOptimize();
                node->GenerateCode(code);
            }
        }
        else if (tk.kind == TokenKind::If)
        {
            MoveNext();
            ASTNode* node = Lexp();
            node = node->CalTypeAndOptimize();
            if (node->varType != VarType::I32)
            {
                ThrowSemantic(tk.filePath, tk.fileLocation, "条件表达式类型必须为i32");
            }
            node->GenerateCode(code);
            SAFE_DELETE(node);

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

            ASTNode* node = Lexp();
            node = node->CalTypeAndOptimize();
            if (node->varType != VarType::I32)
            {
                ThrowSemantic(tk.filePath, tk.fileLocation, "条件表达式类型必须为i32");
            }
            node->GenerateCode(code);
            SAFE_DELETE(node);

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
                const VarInfo& varInfo = GetVar(tkVar, &l);
                if (varInfo.type == VarType::I32)
                {
                    code.emplace_back(ILInstType::RED, l, varInfo.runtimeAddress);
                }
                else if (varInfo.type == VarType::F32)
                {
                    code.emplace_back(ILInstType::FRED, l, varInfo.runtimeAddress);
                }
            } while (TryMatch(TokenKind::Comma));
            Match(TokenKind::RParen);
            Match(TokenKind::Semi);
        }
        else if (tk.kind == TokenKind::Write)
        {
            MoveNext();
            Match(TokenKind::LParen);
            do
            {
                ASTNode* node = Exp();
                node = node->CalTypeAndOptimize();
                node->GenerateCode(code);

                if (node->varType == VarType::I32)
                {
                    code.emplace_back(ILInstType::WRT, 0, 0);
                }
                else if (node->varType == VarType::F32)
                {
                    code.emplace_back(ILInstType::FWRT, 0, 0);
                }
                else
                {
                    ThrowSemantic(tk.filePath, tk.fileLocation, "write()参数类型错误");
                }

                SAFE_DELETE(node);
            } while (TryMatch(TokenKind::Comma));
            Match(TokenKind::RParen);
            Match(TokenKind::Semi);
        }
        else if (tk.kind == TokenKind::Return)
        {
            MoveNext();
            if (TryMatch(TokenKind::Semi))
            {
                if (procedure.ret)
                {
                    ThrowSemantic(tk.filePath, tk.fileLocation, "过程声明了返回值，但实际没有提供");
                }
                GenPopVars();
                code.emplace_back(ILInstType::OPR, 0, static_cast<int32_t>(ILInstOprType::Ret));
            }
            else
            {
                ASTNode* node = Exp();
                Match(TokenKind::Semi);

                if (!procedure.ret)
                {
                    ThrowSemantic(tk.filePath, tk.fileLocation, "过程没有声明返回值");
                }

                node = node->CalTypeAndOptimize();

                if (node->varType != procedure.ret->type)
                {
                    ThrowSemantic(tk.filePath, tk.fileLocation, std::string("返回值类型不匹配，过程声明返回值类型为")
                        + to_string(procedure.ret->type) + "，实际返回的类型为" + to_string(node->varType));
                }
                node->GenerateCode(code);
                SAFE_DELETE(node);

                code.emplace_back(ILInstType::STO, 0, procedure.ret->runtimeAddress);
                GenPopVars();
                code.emplace_back(ILInstType::OPR, 0, static_cast<int32_t>(ILInstOprType::Ret));
            }
        }
        else
        {
            ThrowSyntax(tk.filePath, tk.fileLocation, "Statement错误");
        }
    }

    void GenPopVars()
    {
        auto procedure = path.back();
        if (!procedure->vars.empty())
        {
            code.emplace_back(ILInstType::INT, 0, -static_cast<int32_t>(procedure->vars.size()));
        }
    }

    // <lexp> → <exp> <lop> <exp>|odd <exp>
    // Logical Expression
    ASTNode* Lexp()
    {
        if (auto tk = TryMatch(TokenKind::Odd))
        {
            auto node = Exp();
            return new UnaryNode(UnaryNodeType::Odd, std::move(*tk), node);
        }
        auto node1 = Exp();
        auto nodeLop = Lop();
        auto node2 = Exp();
        nodeLop->left = node1;
        nodeLop->right = node2;
        return nodeLop;
    }

    // exp.first = +, -, id, int, (
    // <exp> -> [+|-]<term>{<aop><term>}
    // Expression
    ASTNode* Exp()
    {
        auto tk = TryMatch(TokenKind::Minus);
        if (!tk)
        {
            TryMatch(TokenKind::Plus);
        }
        auto node1 = Term();
        if (tk)
        {
            node1 = new UnaryNode(UnaryNodeType::Neg, std::move(*tk), node1);
        }
        while (TryCurrent({TokenKind::Plus, TokenKind::Minus}))
        {
            auto nodeAop = Aop();
            auto node2 = Term();
            nodeAop->left = node1;
            nodeAop->right = node2;
            node1 = nodeAop;
        }
        return node1;
    }

    // <term> -> <factor>{<mop><factor>}
    // term.first = id, int, (
    ASTNode* Term()
    {
        auto node1 = Factor();
        while (TryCurrent({TokenKind::Star, TokenKind::Slash}))
        {
            auto nodeMop = Mop();
            auto node2 = Factor();
            nodeMop->left = node1;
            nodeMop->right = node2;
            node1 = nodeMop;
        }
        return node1;
    }

    // <factor> ->  <id>
    //             |<CallProcedure>
    //             |<literal>
    //             |(<exp>)
    //             |i32 (<exp>)
    //             |f32 (<exp>)
    ASTNode* Factor()
    {
        std::vector<TokenKind> excepted {
            TokenKind::Identifier,
            TokenKind::Int32,
            TokenKind::Float32,
            TokenKind::LParen,
            TokenKind::I32,
            TokenKind::F32};
        Token tk = Match(excepted);

        if (tk.kind == TokenKind::Identifier)
        {
            if (TryMatch(TokenKind::LParen))
            {
                // 匹配到过程调用
                return CallProcedure(tk, true);
            }

            int l;
            const VarInfo& varInfo = GetVar(tk, &l);
            if (varInfo.bConst)
            {
                // 常量传播
                return new ValueNode(varInfo.type, varInfo.value);
            }
            return new VarNode(&varInfo, l);
        }
        if (tk.kind == TokenKind::Int32)
        {
            return new ValueNode(VarType::I32, tk.Int32());
        }
        if (tk.kind == TokenKind::Float32)
        {
            return new ValueNode(VarType::F32, tk.Float32());
        }
        if (tk.kind == TokenKind::LParen)
        {
            ASTNode* node = Exp();
            Match(TokenKind::RParen);
            return node;
        }
        if (tk.kind == TokenKind::I32)
        {
            Match(TokenKind::LParen);
            ASTNode* node = Exp();
            Match(TokenKind::RParen);
            return new UnaryNode(UnaryNodeType::ToI32, std::move(tk), node);
        }
        if (tk.kind == TokenKind::F32)
        {
            Match(TokenKind::LParen);
            auto node = Exp();
            Match(TokenKind::RParen);
            return new UnaryNode(UnaryNodeType::ToF32, std::move(tk), node);
        }
        ThrowSyntax(tk.filePath, tk.fileLocation, "Factor错误");
    }

    // <lop> -> =|<>|<|<=|>|>=
    BinaryNode* Lop()
    {
        const auto tk = Match({
            TokenKind::Equal, TokenKind::LessGreater,
            TokenKind::Less, TokenKind::LessEqual,
            TokenKind::Greater, TokenKind::GreaterEqual});
        switch (tk.kind)
        {
        case TokenKind::Equal:
            return new BinaryNode(BinaryNodeType::Eql, std::move(tk));
        case TokenKind::LessGreater:
            return new BinaryNode(BinaryNodeType::Neq, std::move(tk));
        case TokenKind::Less:
            return new BinaryNode(BinaryNodeType::Lss, std::move(tk));
        case TokenKind::LessEqual:
            return new BinaryNode(BinaryNodeType::Leq, std::move(tk));
        case TokenKind::Greater:
            return new BinaryNode(BinaryNodeType::Gtr, std::move(tk));
        case TokenKind::GreaterEqual:
            return new BinaryNode(BinaryNodeType::Geq, std::move(tk));
        default:
            ThrowSyntax(tk.filePath, tk.fileLocation, "Lop错误");
        }
    }

    // <aop> -> +|-
    BinaryNode* Aop()
    {
        const auto tk = Match({TokenKind::Plus, TokenKind::Minus});
        if (tk.kind == TokenKind::Plus)
        {
            return new BinaryNode(BinaryNodeType::Add, std::move(tk));
        }
        return new BinaryNode(BinaryNodeType::Sub, std::move(tk));
    }

    // <mop> -> *|/
    BinaryNode* Mop()
    {
        const auto tk = Match({TokenKind::Star, TokenKind::Slash});
        if (tk.kind == TokenKind::Star)
        {
            return new BinaryNode(BinaryNodeType::Mul, std::move(tk));
        }
        return new BinaryNode(BinaryNodeType::Div, std::move(tk));
    }

    const VarInfo& GetVar(const Token& tkVar, int* pL)
    {
        int l = 0;
        const std::string& name = tkVar.rawText;
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
        ThrowSyntax(tkVar.filePath, tkVar.fileLocation, "未定义变量" + name);
    }
};
