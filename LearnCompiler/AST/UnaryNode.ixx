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

export struct UnaryNode : ASTNode
{
    ASTNode* child;
    UnaryNodeType type;
    Token token;

    explicit UnaryNode(UnaryNodeType type, Token token, ASTNode* child):
    ASTNode(VarType::I32), child(child), type(type), token(std::move(token))
    {
    }

    ASTNode* CalTypeAndOptimize() override
    {
        THROW_IF_NULL(child);
        child = child->CalTypeAndOptimize();

        const auto it = unaryNodeEvaluators.find(UnaryNodeEvaluator(type, child->varType));
        if (it == unaryNodeEvaluators.end())
        {
            ThrowSemantic(std::string(token.filePath), token.fileLocation,
                "不存在定义在类型" + std::string(to_string(type)) +
                "上的一元操作" + std::string(to_string(child->varType)));
        }
        auto p = dynamic_cast<ValueNode*>(child);
        if (p)
        {
            auto p2 = it->eval(p);
            delete this;
            return p2;
        }
        varType = it->ret;
        return this;
    }

    void GenerateCode(std::vector<::ILInst>& code) override
    {
        child->GenerateCode(code);
        auto it = unaryNodeEvaluators.find(UnaryNodeEvaluator(type, child->varType));
        code.push_back(it->inst);
    }

    ~UnaryNode() override
    {
        delete child;
    }
};