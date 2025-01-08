module;
#include <stdexcept>
#include "../PtrHelper.h"
#include <vector>

export module UnaryNode;

import ASTNode;
import UnaryNodeEvaluator;
import UnaryNodeType;
import ValueNode;
import ILInst;
import VarInfo;
import Token;
import ErrorUtils;

// 定义一元操作节点结构体，继承自ASTNode
export struct UnaryNode : ASTNode
{
    // 子节点指针
    ASTNode* child;
    // 一元操作类型
    UnaryNodeType type;
    // 关联的词法符号
    Token token;

    // 构造函数
    explicit UnaryNode(UnaryNodeType type, Token token, ASTNode* child):
    ASTNode(VarType::I32), child(child), type(type), token(std::move(token))
    {
    }

    // 计算类型并优化节点
    ASTNode* CalTypeAndOptimize() override
    {
        // 确保子节点不为空
        THROW_IF_NULL(child);
        // 递归计算和优化子节点
        child = child->CalTypeAndOptimize();

        // 查找一元操作的评估器
        const auto it = unaryNodeEvaluators.find(UnaryNodeEvaluator(type, child->varType));
        // 如果找不到对应的评估器，抛出语义错误
        if (it == unaryNodeEvaluators.end())
        {
            ThrowSemantic(std::string(token.filePath), token.fileLocation,
                "不存在定义在类型" + std::string(to_string(type)) +
                "上的一元操作" + std::string(to_string(child->varType)));
        }
        // 如果子节点是值节点，尝试进行常量折叠优化
        auto p = dynamic_cast<ValueNode*>(child);
        if (p)
        {
            auto p2 = it->eval(p);
            delete this;
            return p2;
        }
        // 更新节点的变量类型
        varType = it->ret;
        return this;
    }

    // 生成中间语言代码
    void GenerateCode(std::vector<::ILInst>& code) override
    {
        // 递归生成子节点的代码
        child->GenerateCode(code);
        // 查找一元操作的中间语言指令
        auto it = unaryNodeEvaluators.find(UnaryNodeEvaluator(type, child->varType));
        // 将指令添加到代码序列中
        code.push_back(it->inst);
    }

    // 析构函数
    ~UnaryNode() override
    {
        delete child;
    }
};