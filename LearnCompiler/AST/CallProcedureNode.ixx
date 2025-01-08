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

// CallProcedureNode类继承自ASTNode，表示一个过程调用
export struct CallProcedureNode : ASTNode
{
    // 指向被调用过程的信息
    ProcedureInfo* proc;
    // 过程调用的层次
    int l;
    // 过程调用的实际参数列表
    std::vector<ASTNode*> args;
    // 是否需要返回值
    bool needRet;
    // 过程调用的令牌信息
    Token token;

    // 构造函数初始化CallProcedureNode对象
    explicit CallProcedureNode(ProcedureInfo* proc, int l, std::vector<ASTNode*> args, bool needRet, Token token):
    ASTNode(proc->ret ? proc->ret->type : VarType::Void),
    proc(proc),
    l(l),
    args(std::move(args)),
    needRet(needRet),
    token(std::move(token))
    {
    }

    // 计算类型并进行优化
    ASTNode* CalTypeAndOptimize() override
    {
        // 检查参数数量是否匹配
        if (args.size() != proc->params.size())
        {
            ThrowSemantic(token.filePath, token.fileLocation,
                std::string("调用过程") + token.rawText +
                "参数个数不匹配，" +
                "形参有" + std::to_string(proc->params.size()) + "个，" +
                "实参有" + std::to_string(args.size()) + "个");
        }

        // 优化每个参数
        for (auto& arg : args)
        {
            arg = arg->CalTypeAndOptimize();
        }

        // 检查参数类型是否匹配
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

    // 生成中间语言代码
    void GenerateCode(std::vector<ILInst>& code) override
    {
        int reserveForRet;
        // 如果过程有返回值，预留空间
        if (proc->ret)
        {
            reserveForRet = 1;
            code.emplace_back(ILInstType::INT, 0, 1);
        }
        else
        {
            reserveForRet = 0;
            // 如果过程没有返回值，但需要返回值，抛出错误
            if (needRet)
            {
                ThrowSemantic(token.filePath, token.fileLocation,
                    "需要返回值，但过程" + token.rawText + "没有返回值");
            }
        }

        // 生成每个参数的代码
        for (auto& arg : args)
        {
            arg->GenerateCode(code);
        }

        // 生成调用过程的代码
        code.emplace_back(ILInstType::CAL, l, proc->codeAddress);

        // 根据是否需要返回值，调整栈
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

    // 析构函数，释放参数列表的内存
    ~CallProcedureNode() override
    {
        for (auto& arg : args)
        {
            delete arg;
        }
    }
};