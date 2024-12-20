module;
#include <vector>
#include <stdexcept>

export module CallProcedureNode;
import ASTNode;
import ProcedureInfo;
import VarInfo;
import ILInst;
import ILInstType;

export struct CallProcedureNode : ASTNode
{
    ProcedureInfo* proc;
    int l;
    std::vector<ASTNode*> args;
    bool needRet;

    explicit CallProcedureNode(ProcedureInfo* proc, int l, std::vector<ASTNode*> args, bool needRet):
    ASTNode(proc->ret ? proc->ret->type : VarType::Void),
    proc(proc), l(l),
    args(std::move(args)),
    needRet(needRet)
    {
    }

    ASTNode* CalTypeAndOptimize() override
    {
        if (args.size() != proc->params.size())
        {
            throw std::runtime_error(
                "过程参数个数不匹配");
        }

        for (auto& arg : args)
        {
            arg = arg->CalTypeAndOptimize();
        }

        for (size_t i = 0; i < args.size(); i++)
        {
            if (args[i]->varType != proc->params[i].type)
            {
                throw std::runtime_error(
                    "过程参数类型不匹配");
            }
        }
        return this;
    }

    void GenerateCode(std::vector<ILInst>& code) override
    {
        for (auto& arg : args)
        {
            arg->GenerateCode(code);
        }

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
                throw std::runtime_error("过程需要返回值");
            }
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
