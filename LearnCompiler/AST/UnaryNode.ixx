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

export struct UnaryNode : ASTNode
{
    ASTNode* child;
    UnaryNodeType type;

    explicit UnaryNode(UnaryNodeType type, ASTNode* child):
    ASTNode(VarType::I32), child(child), type(type)
    {
    }

    ASTNode* CalTypeAndOptimize() override
    {
        THROW_IF_NULL(child);
        child = child->CalTypeAndOptimize();

        const auto it = unaryNodeEvaluators.find(UnaryNodeEvaluator(type, child->varType));
        if (it == unaryNodeEvaluators.end())
        {
            throw std::runtime_error("不存在操作" + std::string(to_string(child->varType)) + " " +
                std::string(to_string(type)));
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