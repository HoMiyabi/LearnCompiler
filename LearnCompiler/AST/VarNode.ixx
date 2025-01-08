module;
#include <vector>

export module VarNode;
import ASTNode; 
import VarInfo; 
import ILInst; 
import ILInstType; 

// VarNode类定义，继承自ASTNode类，表示AST中的变量节点
export struct VarNode : ASTNode
{
    const VarInfo* varInfo; // 指向变量信息的指针，用于存储变量的类型和地址等信息
    int l; // 变量的生命周期标识符

    // 构造函数，显式指定varInfo和l参数
    explicit VarNode(const VarInfo* varInfo, const int l):
    ASTNode(varInfo->type), varInfo(varInfo), l(l)
    {
    }

    // 重写GenerateCode函数，生成中间语言指令
    void GenerateCode(std::vector<ILInst>& code) override
    {
        // 生成LOD指令，用于加载变量的运行时地址到栈中
        code.emplace_back(ILInstType::LOD, l, varInfo->runtimeAddress);
    }
};