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

export struct BinaryNode : ASTNode
{
    ASTNode* left;
    ASTNode* right;
    BinaryNodeType type;
    Token token;

    explicit BinaryNode(BinaryNodeType type, Token token, ASTNode* left = nullptr, ASTNode* right = nullptr):
    ASTNode(VarType::I32), left(left), right(right), type(type), token(std::move(token))
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
            ThrowSemantic(std::string(token.filePath), token.fileLocation,
                "不存在定义在类型" + std::string(to_string(left->varType)) + "和类型" +
                std::string(to_string(right->varType)) + "上的二元操作" + std::string(to_string(type)));
        }
        auto l = dynamic_cast<ValueNode*>(left);
        auto r = dynamic_cast<ValueNode*>(right);
        if (l && r)
        {
            auto p = it->eval(l, r);
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
