module;
#include <vector>


export module ASTNode;
import VarInfo;
import ILInst;

// 定义ASTNode基类，表示抽象语法树节点
export struct ASTNode
{
    // 成员变量：存储变量类型信息
    VarType varType;

    // 构造函数：初始化varType
    explicit ASTNode(VarType varType): varType(varType)
    {
    }

    // 虚拟函数：计算类型并优化AST节点
    // 返回值：优化后的AST节点指针
    virtual ASTNode* CalTypeAndOptimize()
    {
        return this;
    }

    // 纯虚函数：生成中间语言代码
    // 参数code：存储生成的中间语言指令的容器
    virtual void GenerateCode(std::vector<ILInst>& code) = 0;

    virtual ~ASTNode()
    {
    }
};