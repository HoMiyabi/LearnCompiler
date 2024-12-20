module;
#include <iostream>
#include <vector>
#include <optional>
#include <variant>

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

export class Parser
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
            "预期" + std::string(to_string(expected)) +
            "，但实际为" + tk.ToString());
    }

    [[noreturn]]
    static void ThrowMismatch(const Token& tk, const std::vector<TokenKind>& expected)
    {
        std::string message = GetErrorPrefix(tk.fileLocation) + "预期";
        for (auto it = expected.begin(); it != expected.end(); ++it)
        {
            message += to_string(*it);
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
        std::string message = "预期" + std::string(to_string(excepted)) + "，但已经到达文件尾部";
        throw std::runtime_error(message);
    }

    [[noreturn]]
    static void ThrowEOF(const std::vector<TokenKind>& excepted)
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

    const Token& Current(const std::vector<TokenKind>& excepted)
    {
        if (!token)
        {
            ThrowEOF(excepted);
        }

        for (auto it = excepted.begin(); it != excepted.end(); ++it)
        {
            if (token->kind == *it)
            {
                token = tokenizer.GetToken();
                return *token;
            }
        }
        ThrowMismatch(*token, excepted);
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

    void GenConst(const Token& tkId, const Token& tkLiteral)
    {
        ProcedureInfo& procedure = *path.back();
        if (ProcedureContainsVar(procedure, tkId.String()))
        {
            throw std::runtime_error(GetErrorPrefix(tkId.fileLocation) + tkId.String() + "重复定义");
        }

        VarInfo varInfo(true, VarType::I32, tkId.String(), procedure.vars.size());
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

    // <vardecl> -> var <id>:<id>{,<id>:<id>};
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

    void GenOneVardecl(const Token& tkId, const Token& tkType)
    {
        ProcedureInfo& procedure = *path.back();
        if (ProcedureContainsVar(procedure, tkId.String()))
        {
            throw std::runtime_error(GetErrorPrefix(tkId.fileLocation) + tkId.String() + "重复定义");
        }
        VarInfo varInfo(false, VarType::I32, tkId.String(), procedure.vars.size());
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
                VarInfo varInfo(false, VarType::I32, tkId.String(), params.size());
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
        return new CallProcedureNode(&proc, l, std::move(args), needRet);
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
            auto tk1 = Match({TokenKind::ColonEqual, TokenKind::LParen});
            if (tk1.kind == TokenKind::ColonEqual)
            {
                auto node = Exp();
                Match(TokenKind::Semi);

                node = node->CalTypeAndOptimize();
                int l;
                const VarInfo& varInfo = GetVar(tk, &l);
                if (node->varType != varInfo.type)
                {
                    throw std::runtime_error(GetErrorPrefix(tk.fileLocation) +
                        "不能将" + to_string(node->varType) + "赋值给" + to_string(varInfo.type));
                }
                node->GenerateCode(code);
                delete node;
                code.emplace_back(ILInstType::STO, l, varInfo.runtimeAddress);
            }
            else if (tk1.kind == TokenKind::LParen)
            {
                CallProcedure(tk, false);
                Match(TokenKind::Semi);
            }
        }
        else if (tk.kind == TokenKind::If)
        {
            MoveNext();
            ASTNode* node = Lexp();
            node = node->CalTypeAndOptimize();
            if (node->varType != VarType::I32)
            {
                throw std::runtime_error(GetErrorPrefix(tk.fileLocation) + "条件表达式类型必须为i32");
            }
            node->GenerateCode(code);
            delete node;

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
                delete node;

                if (node->varType == VarType::I32)
                {
                    code.emplace_back(ILInstType::WRT, 0, 0);
                }
                else if (node->varType == VarType::F32)
                {
                    code.emplace_back(ILInstType::FWRT, 0, 0);
                }
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
                    throw std::runtime_error(GetErrorPrefix(tk.fileLocation) + "过程需要返回值");
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
                    throw std::runtime_error(GetErrorPrefix(tk.fileLocation) + "过程没有返回值");
                }

                node = node->CalTypeAndOptimize();

                if (node->varType != procedure.ret->type)
                {
                    throw std::runtime_error(GetErrorPrefix(tk.fileLocation) + "返回值类型不匹配");
                }
                node->GenerateCode(code);
                delete node;

                code.emplace_back(ILInstType::STO, 0, procedure.ret->runtimeAddress);
                GenPopVars();
                code.emplace_back(ILInstType::OPR, 0, static_cast<int32_t>(ILInstOprType::Ret));
            }
        }
        else
        {
            throw std::runtime_error(GetErrorPrefix(tk.fileLocation) + "Statement错误");
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
            return new UnaryNode(UnaryNodeType::Odd, node);
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
        bool flag = false;
        if (TryMatch(TokenKind::Minus))
        {
            flag = true;
        }
        else
        {
            TryMatch(TokenKind::Plus);
        }
        auto node1 = Term();
        if (flag)
        {
            node1 = new UnaryNode(UnaryNodeType::Neg, node1);
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
        const Token tk = Match(excepted);

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
            return new UnaryNode(UnaryNodeType::ToI32, node);
        }
        if (tk.kind == TokenKind::F32)
        {
            Match(TokenKind::LParen);
            auto node = Exp();
            Match(TokenKind::RParen);
            return new UnaryNode(UnaryNodeType::ToF32, node);
        }
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
            return new BinaryNode(BinaryNodeType::Eql);
        case TokenKind::LessGreater:
            return new BinaryNode(BinaryNodeType::Neq);
        case TokenKind::Less:
            return new BinaryNode(BinaryNodeType::Lss);
        case TokenKind::LessEqual:
            return new BinaryNode(BinaryNodeType::Leq);
        case TokenKind::Greater:
            return new BinaryNode(BinaryNodeType::Gtr);
        case TokenKind::GreaterEqual:
            return new BinaryNode(BinaryNodeType::Geq);
        default:
            throw std::runtime_error(GetErrorPrefix(tk.fileLocation) + "Lop错误");
        }
    }

    // <aop> -> +|-
    BinaryNode* Aop()
    {
        const auto tk = Match({TokenKind::Plus, TokenKind::Minus});
        if (tk.kind == TokenKind::Plus)
        {
            return new BinaryNode(BinaryNodeType::Add);
        }
        return new BinaryNode(BinaryNodeType::Sub);
    }

    // <mop> -> *|/
    BinaryNode* Mop()
    {
        const auto tk = Match({TokenKind::Star, TokenKind::Slash});
        if (tk.kind == TokenKind::Star)
        {
            return new BinaryNode(BinaryNodeType::Mul);
        }
        return new BinaryNode(BinaryNodeType::Div);
    }

    const VarInfo& GetVar(const Token& tkVar, int* pL)
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
