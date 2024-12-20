module;
#include <stdexcept>
#include "ThrowHelper.h"
#include <vector>

export module BinaryNode;
import BinaryOp;
import ASTNode;
import BinaryNodeType;
import ValueNode;
import VarInfo;
import ILInst;

export struct BinaryNode : ASTNode
{
    ASTNode* left;
    ASTNode* right;
    BinaryNodeType type;

    explicit BinaryNode(BinaryNodeType type, ASTNode* left = nullptr, ASTNode* right = nullptr):
    ASTNode(VarType::I32), left(left), right(right), type(type)
    {
    }

    ASTNode* CalTypeAndOptimize() override
    {
        THROW_IF_NULL(left);
        THROW_IF_NULL(right);
        left = left->CalTypeAndOptimize();
        right = right->CalTypeAndOptimize();

        const auto it = builtInBinaryOps.find(BuiltInBinaryOp(type, left->varType, right->varType));
        if (it == builtInBinaryOps.end())
        {
            throw std::runtime_error("不存在操作" + std::string(to_string(left->varType)) + " " +
                std::string(to_string(type)) + " " +
                std::string(to_string(right->varType)));
        }
        auto l = dynamic_cast<ValueNode*>(left);
        auto r = dynamic_cast<ValueNode*>(right);
        if (l && r)
        {
            auto p = it->eval(l, r);
            delete l;
            delete r;
            delete this;
            return p;
        }
        varType = it->ret;
        return this;
    }

    void GenerateCode(std::vector<ILInst>& code) override
    {
        left->GenerateCode(code);
        right->GenerateCode(code);
        auto it = builtInBinaryOps.find(BuiltInBinaryOp(type, left->varType, right->varType));
        code.push_back(it->inst);
    }

    ~BinaryNode() override
    {
        delete left;
        delete right;
    }
};
