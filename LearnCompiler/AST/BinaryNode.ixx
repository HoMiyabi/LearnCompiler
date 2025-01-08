module;
#include <stdexcept>
#include "../PtrHelper.h"
#include <vector>

export module BinaryNode;

import BinaryOp;
import ASTNode;
import BinaryNodeType;
import ValueNode;
import VarInfo;
import ILInst;
import Token;
import ErrorUtils;

// 定义二元操作节点结构体，继承自ASTNode
export struct BinaryNode : ASTNode
{
    // 指向左操作数的指针
    ASTNode* left;
    // 指向右操作数的指针
    ASTNode* right;
    // 二元操作的类型
    BinaryNodeType type;
    // 与二元操作相关的词法符号
    Token token;

    // 构造函数
    explicit BinaryNode(BinaryNodeType type, Token token, ASTNode* left = nullptr, ASTNode* right = nullptr):
    ASTNode(VarType::I32), left(left), right(right), type(type), token(std::move(token))
    {
    }

    // 计算类型并优化的函数
    ASTNode* CalTypeAndOptimize() override
    {
        // 确保左操作数不为空
        THROW_IF_NULL(left);
        // 确保右操作数不为空
        THROW_IF_NULL(right);
        // 优化左操作数
        left = left->CalTypeAndOptimize();
        // 优化右操作数
        right = right->CalTypeAndOptimize();

        // 查找内置二元操作
        const auto it = builtInBinaryOps.find(BuiltInBinaryOp(type, left->varType, right->varType));
        // 如果找不到对应的内置二元操作，则抛出语义错误
        if (it == builtInBinaryOps.end())
        {
            ThrowSemantic(std::string(token.filePath), token.fileLocation,
                "不存在定义在类型" + std::string(to_string(left->varType)) + "和类型" +
                std::string(to_string(right->varType)) + "上的二元操作" + std::string(to_string(type)));
        }
        // 尝试将左操作数和右操作数转换为值节点
        auto l = dynamic_cast<ValueNode*>(left);
        auto r = dynamic_cast<ValueNode*>(right);
        // 如果左右操作数都是值节点，则执行优化
        if (l && r)
        {
            auto p = it->eval(l, r);
            delete this;
            return p;
        }
        // 设置变量类型为操作的结果类型
        varType = it->ret;
        return this;
    }

    // 生成中间语言代码的函数
    void GenerateCode(std::vector<ILInst>& code) override
    {
        // 生成左操作数的代码
        left->GenerateCode(code);
        // 生成右操作数的代码
        right->GenerateCode(code);
        // 查找内置二元操作
        auto it = builtInBinaryOps.find(BuiltInBinaryOp(type, left->varType, right->varType));
        // 添加相应的中间语言指令到代码向量中
        code.push_back(it->inst);
    }

    // 析构函数
    ~BinaryNode() override
    {
        delete left;
        delete right;
    }
};
