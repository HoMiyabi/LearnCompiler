module;
#include <vector>

export module ASTNode;
import VarInfo;
import ILInst;

export struct ASTNode
{
    VarType varType;

    explicit ASTNode(VarType varType): varType(varType)
    {
    }

    virtual ASTNode* CalTypeAndOptimize() = 0;
    virtual void GenerateCode(std::vector<ILInst>& code) = 0;
    virtual ~ASTNode()
    {
    }
};