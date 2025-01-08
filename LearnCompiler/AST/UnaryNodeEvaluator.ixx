module;
#include <unordered_set>
#include <variant>

export module UnaryNodeEvaluator;
import ValueNode;
import UnaryNodeType;
import VarInfo;
import ILInst;
import ILInstOprType;

// 定义一元节点评估器结构体
export struct UnaryNodeEvaluator
{
    // 类型枚举，表示一元操作的类型
    UnaryNodeType type;
    // 操作数的类型
    VarType t;
    // 返回值的类型，默认为I32
    VarType ret = VarType::I32;

    // 中间语言指令，表示一元操作的IL代码
    ILInst inst{};
    // 评估函数指针，用于执行一元操作
    ValueNode* (*eval)(ValueNode*) = nullptr;

    // 构造函数，初始化type和t
    UnaryNodeEvaluator(UnaryNodeType type, VarType t):
    type(type), t(t)
    {
    }

    // 构造函数，初始化所有成员变量
    UnaryNodeEvaluator(UnaryNodeType type, VarType t, VarType ret, ILInst inst, ValueNode* (*eval)(ValueNode*)):
    type(type), t(t), ret(ret), inst(inst), eval(eval)
    {
    }

    // 重载等号运算符，用于比较两个一元节点评估器是否相等
    bool operator==(const UnaryNodeEvaluator& rhs) const
    {
        return type == rhs.type && t == rhs.t;
    }
};

// 定义一元节点评估器的哈希函数
export struct UnaryNodeEvaluatorHasher
{
    // 哈希函数，根据一元节点评估器的成员变量计算哈希值
    std::size_t operator()(const UnaryNodeEvaluator& op) const
    {
        std::hash<int32_t> h;
        return
            h(static_cast<int32_t>(op.type)) ^
            h(static_cast<int32_t>(op.t));
    }
};

// 以下是一元操作的评估函数
ValueNode* EvalNeg(ValueNode* node)
{
    // 评估负数操作，返回一个新的值节点，其值为原节点值的负数
    return new ValueNode(VarType::I32, -std::get<int32_t>(node->value));
}

ValueNode* EvalFNeg(ValueNode* node)
{
    // 评估浮点数负数操作，返回一个新的值节点，其值为原节点浮点值的负数
    return new ValueNode(VarType::F32, -std::get<float>(node->value));
}

ValueNode* EvalOdd(ValueNode* node)
{
    // 评估奇数操作，返回一个新的值节点，其值为原节点值是否为奇数
    return new ValueNode(VarType::I32, std::get<int32_t>(node->value) % 2);
}

ValueNode* EvalToI32(ValueNode* node)
{
    // 评估转换为I32操作，返回一个新的值节点，其值为原节点浮点值转换为整数
    return new ValueNode(VarType::I32, static_cast<int32_t>(std::get<float>(node->value)));
}

ValueNode* EvalToF32(ValueNode* node)
{
    // 评估转换为F32操作，返回一个新的值节点，其值为原节点整数值转换为浮点数
    return new ValueNode(VarType::F32, static_cast<float>(std::get<int32_t>(node->value)));
}

// 导出一元节点评估器的集合，包含所有支持的一元操作
export std::unordered_set<UnaryNodeEvaluator, UnaryNodeEvaluatorHasher> unaryNodeEvaluators
{
    {UnaryNodeType::Neg, VarType::I32, VarType::I32, ILInst(ILInstOprType::Neg), EvalNeg},
    {UnaryNodeType::Neg, VarType::F32, VarType::F32, ILInst(ILInstOprType::FNeg), EvalFNeg},
    {UnaryNodeType::Odd, VarType::I32, VarType::I32, ILInst(ILInstOprType::Odd), EvalOdd},
    {UnaryNodeType::ToI32, VarType::F32, VarType::I32, ILInst(ILInstOprType::FToI), EvalToI32},
    {UnaryNodeType::ToF32, VarType::I32, VarType::F32, ILInst(ILInstOprType::IToF), EvalToF32},
};