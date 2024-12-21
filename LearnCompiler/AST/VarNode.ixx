module;
#include <vector>

export module VarNode;
import ASTNode;
import VarInfo;
import ILInst;
import ILInstType;

export struct VarNode : ASTNode
{
    const VarInfo* varInfo;
    int l;

    explicit VarNode(const VarInfo* varInfo, const int l):
    ASTNode(varInfo->type), varInfo(varInfo), l(l)
    {
    }

    void GenerateCode(std::vector<ILInst>& code) override
    {
        code.emplace_back(ILInstType::LOD, 0, varInfo->runtimeAddress);
    }
};
