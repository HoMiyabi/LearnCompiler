module;
#include <vector>
#include <stdexcept>
#include <string>

export module CallProcedureNode;
import ASTNode;
import ProcedureInfo;
import VarInfo;
import ILInst;
import ILInstType;
import ErrorUtils;
import Token;

export struct CallProcedureNode : ASTNode
{
    ProcedureInfo* proc;
    int l;
    std::vector<ASTNode*> args;
    bool needRet;
    Token token;

    explicit CallProcedureNode(ProcedureInfo* proc, int l, std::vector<ASTNode*> args, bool needRet, Token token):
    ASTNode(proc->ret ? proc->ret->type : VarType::Void),
    proc(proc),
    l(l),
    args(std::move(args)),
    needRet(needRet),
    token(std::move(token))
    {
    }

    ASTNode* CalTypeAndOptimize() override
    {
        if (args.size() != proc->params.size())
        {
            ThrowSemantic(token.filePath, token.fileLocation,
                std::string("调用过程") + token.rawText +
                "参数个数不匹配，" +
                "形参有" + std::to_string(proc->params.size()) + "个，" +
                "实参有" + std::to_string(args.size()) + "个");
        }

        for (auto& arg : args)
        {
            arg = arg->CalTypeAndOptimize();
        }

        for (size_t i = 0; i < args.size(); i++)
        {
            if (args[i]->varType != proc->params[i].type)
            {
                ThrowSemantic(token.filePath, token.fileLocation,
                    std::string("调用过程") + token.rawText +
                    "第" + std::to_string(i + 1) + "个参数类型不匹配，" +
                    "形参类型为" + to_string(proc->params[i].type) + "，" +
                    "实参类型为" + to_string(args[i]->varType));
            }
        }
        return this;
    }

    void GenerateCode(std::vector<ILInst>& code) override
    {
        int reserveForRet;
        if (proc->ret)
        {
            reserveForRet = 1;
            code.emplace_back(ILInstType::INT, 0, 1);
        }
        else
        {
            reserveForRet = 0;
            if (needRet)
            {
                ThrowSemantic(token.filePath, token.fileLocation,
                    "需要返回值，但过程" + token.rawText + "没有返回值");
            }
        }

        for (auto& arg : args)
        {
            arg->GenerateCode(code);
        }

        code.emplace_back(ILInstType::CAL, l, proc->codeAddress);

        if (needRet)
        {
            if (args.size() != 0)
            {
                code.emplace_back(ILInstType::INT, 0, -static_cast<int>(args.size()));
            }
        }
        else
        {
            if ((reserveForRet + args.size()) != 0)
            {
                code.emplace_back(ILInstType::INT, 0, -static_cast<int>(reserveForRet + args.size()));
            }
        }
    }

    ~CallProcedureNode() override
    {
        for (auto& arg : args)
        {
            delete arg;
        }
    }
};
