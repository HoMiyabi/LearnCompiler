module;
#include <variant>
#include <vector>

export module ValueNode;
import ASTNode;
import ILInst;
import VarInfo;

// ValueNode类继承自ASTNode，表示AST中的值节点
export struct ValueNode : ASTNode
{
    // 值节点可以是32位整数或32位浮点数
    std::variant<int32_t, float> value;

    // 构造函数，初始化值节点
    // 参数varType：变量类型，用于确定value的类型
    // 参数value：值节点的值，可以是int32_t或float
    explicit ValueNode(VarType varType, std::variant<int32_t, float> value):
    ASTNode(varType), value(value)
    {
    }

    // 生成中间语言代码
    // 参数code：ILInst类型的向量，用于存储生成的中间语言指令
    void GenerateCode(std::vector<ILInst>& code) override
    {
        // 根据值节点的类型，生成相应的中间语言指令
        if (varType == VarType::I32)
        {
            // 如果是整数类型，生成加载整数常量的指令
            code.push_back(ILInst::LIT(std::get<int32_t>(value)));
        }
        else if (varType == VarType::F32)
        {
            // 如果是浮点数类型，生成加载浮点数常量的指令
            code.push_back(ILInst::LIT(std::get<float>(value)));
        }
    };
};