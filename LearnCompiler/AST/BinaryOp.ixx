module;
#include <unordered_set>
#include <variant>

export module BinaryOp;
import ValueNode;
import BinaryNodeType;
import VarInfo;
import ILInst;
import ILInstOprType;

// 导出类 BuiltInBinaryOp，用于表示内置的二元操作符
export class BuiltInBinaryOp
{
public:
    BinaryNodeType op; // 操作符类型
    VarType t1; // 第一个操作数的类型
    VarType t2; // 第二个操作数的类型

    VarType ret = VarType::I32; // 返回值类型，默认为 I32
    ILInst inst{}; // 中间代码指令
    ValueNode* (*eval)(ValueNode*, ValueNode*) = nullptr; // 评估函数指针

    // 构造函数，初始化操作符类型和操作数类型
    BuiltInBinaryOp(BinaryNodeType op, VarType t1, VarType t2):
        op(op), t1(t1), t2(t2)
    {
    }

    // 构造函数，初始化操作符类型、操作数类型、返回值类型、中间代码指令和评估函数
    BuiltInBinaryOp(BinaryNodeType op, VarType t1, VarType t2, VarType ret, ILInst inst,
                    ValueNode* (*eval)(ValueNode*, ValueNode*)):
        op(op), t1(t1), t2(t2), ret(ret), inst(inst), eval(eval)
    {
    }

    // 重载 == 运算符，比较两个 BuiltInBinaryOp 对象是否相等
    bool operator==(const BuiltInBinaryOp& other) const
    {
        return op == other.op && t1 == other.t1 && t2 == other.t2;
    }
};

// 结构体 BuiltInBinaryOpHasher，用于计算 BuiltInBinaryOp 对象的哈希值
export struct BuiltInBinaryOpHasher
{
    std::size_t operator()(const BuiltInBinaryOp& op) const
    {
        std::hash<int32_t> h;
        return
            h(static_cast<int32_t>(op.op)) ^ // 计算操作符类型的哈希值
            h(static_cast<int32_t>(op.t1)) ^ // 计算第一个操作数类型的哈希值
            h(static_cast<int32_t>(op.t2)); // 计算第二个操作数类型的哈希值
    }
};

// 以下为各种二元操作符的评估函数实现

// i32 类型的加法操作
ValueNode* EvalAdd(ValueNode* node1, ValueNode* node2)
{
    int32_t value = std::get<int32_t>(node1->value) + std::get<int32_t>(node2->value);
    return new ValueNode(VarType::I32, value);
}

// i32 类型的减法操作
ValueNode* EvalSub(ValueNode* node1, ValueNode* node2)
{
    int32_t value = std::get<int32_t>(node1->value) - std::get<int32_t>(node2->value);
    return new ValueNode(VarType::I32, value);
}

// i32 类型的乘法操作
ValueNode* EvalMul(ValueNode* node1, ValueNode* node2)
{
    int32_t value = std::get<int32_t>(node1->value) * std::get<int32_t>(node2->value);
    return new ValueNode(VarType::I32, value);
}

// i32 类型的除法操作
ValueNode* EvalDiv(ValueNode* node1, ValueNode* node2)
{
    int32_t value = std::get<int32_t>(node1->value) / std::get<int32_t>(node2->value);
    return new ValueNode(VarType::I32, value);
}

// i32 类型的等于操作
ValueNode* EvalEql(ValueNode* node1, ValueNode* node2)
{
    int32_t value = std::get<int32_t>(node1->value) == std::get<int32_t>(node2->value);
    return new ValueNode(VarType::I32, value);
}

// i32 类型的不等于操作
ValueNode* EvalNeq(ValueNode* node1, ValueNode* node2)
{
    int32_t value = std::get<int32_t>(node1->value) != std::get<int32_t>(node2->value);
    return new ValueNode(VarType::I32, value);
}

// i32 类型的小于操作
ValueNode* EvalLss(ValueNode* node1, ValueNode* node2)
{
    int32_t value = std::get<int32_t>(node1->value) < std::get<int32_t>(node2->value);
    return new ValueNode(VarType::I32, value);
}

// i32 类型的小于等于操作
ValueNode* EvalLeq(ValueNode* node1, ValueNode* node2)
{
    int32_t value = std::get<int32_t>(node1->value) <= std::get<int32_t>(node2->value);
    return new ValueNode(VarType::I32, value);
}

// i32 类型的大于操作
ValueNode* EvalGtr(ValueNode* node1, ValueNode* node2)
{
    int32_t value = std::get<int32_t>(node1->value) > std::get<int32_t>(node2->value);
    return new ValueNode(VarType::I32, value);
}

// i32 类型的大于等于操作
ValueNode* EvalGeq(ValueNode* node1, ValueNode* node2)
{
    int32_t value = std::get<int32_t>(node1->value) >= std::get<int32_t>(node2->value);
    return new ValueNode(VarType::I32, value);
}

// f32 类型的加法操作
ValueNode* EvalFAdd(ValueNode* node1, ValueNode* node2)
{
    float value = std::get<float>(node1->value) + std::get<float>(node2->value);
    return new ValueNode(VarType::F32, value);
}

// f32 类型的减法操作
ValueNode* EvalFSub(ValueNode* node1, ValueNode* node2)
{
    float value = std::get<float>(node1->value) - std::get<float>(node2->value);
    return new ValueNode(VarType::F32, value);
}

// f32 类型的乘法操作
ValueNode* EvalFMul(ValueNode* node1, ValueNode* node2)
{
    float value = std::get<float>(node1->value) * std::get<float>(node2->value);
    return new ValueNode(VarType::F32, value);
}

//执行两个值节点的浮点数除法操作
ValueNode* EvalFDiv(ValueNode* node1, ValueNode* node2)
{
    float value = std::get<float>(node1->value) / std::get<float>(node2->value);
    return new ValueNode(VarType::F32, value);
}


//比较两个值节点的浮点数是否相等
ValueNode* EvalFEql(ValueNode* node1, ValueNode* node2)
{
    int32_t value = std::get<float>(node1->value) == std::get<float>(node2->value);
    return new ValueNode(VarType::I32, value);
}


//比较两个值节点的浮点数是否不相等
ValueNode* EvalFNeq(ValueNode* node1, ValueNode* node2)
{
    int32_t value = std::get<float>(node1->value) != std::get<float>(node2->value);
    return new ValueNode(VarType::I32, value);
}

//比较两个值节点的浮点数大小，判断第一个是否小于第二个
ValueNode* EvalFLss(ValueNode* node1, ValueNode* node2)
{
    int32_t value = std::get<float>(node1->value) < std::get<float>(node2->value);
    return new ValueNode(VarType::I32, value);
}

//比较两个值节点的浮点数大小，判断第一个是否小于等于第二个
ValueNode* EvalFLeq(ValueNode* node1, ValueNode* node2)
{
    int32_t value = std::get<float>(node1->value) <= std::get<float>(node2->value);
    return new ValueNode(VarType::I32, value);
}

//比较两个值节点的浮点数大小，判断第一个是否大于第二个
ValueNode* EvalFGtr(ValueNode* node1, ValueNode* node2)
{
    int32_t value = std::get<float>(node1->value) > std::get<float>(node2->value);
    return new ValueNode(VarType::I32, value);
}

//比较两个值节点的浮点数大小，判断第一个是否大于等于第二个
ValueNode* EvalFGeq(ValueNode* node1, ValueNode* node2)
{
    int32_t value = std::get<float>(node1->value) >= std::get<float>(node2->value);
    return new ValueNode(VarType::I32, value);
}

// 定义内置的二元操作集合，用于在中间语言层面上处理不同类型的操作数
export std::unordered_set<BuiltInBinaryOp, BuiltInBinaryOpHasher> builtInBinaryOps
{
    // 整数类型二元操作
    {BinaryNodeType::Add, VarType::I32, VarType::I32, VarType::I32, ILInst(ILInstOprType::Add), EvalAdd},
    {BinaryNodeType::Sub, VarType::I32, VarType::I32, VarType::I32, ILInst(ILInstOprType::Sub), EvalSub},
    {BinaryNodeType::Mul, VarType::I32, VarType::I32, VarType::I32, ILInst(ILInstOprType::Mul), EvalMul},
    {BinaryNodeType::Div, VarType::I32, VarType::I32, VarType::I32, ILInst(ILInstOprType::Div), EvalDiv},
    {BinaryNodeType::Eql, VarType::I32, VarType::I32, VarType::I32, ILInst(ILInstOprType::Eql), EvalEql},
    {BinaryNodeType::Neq, VarType::I32, VarType::I32, VarType::I32, ILInst(ILInstOprType::Neq), EvalNeq},
    {BinaryNodeType::Lss, VarType::I32, VarType::I32, VarType::I32, ILInst(ILInstOprType::Lss), EvalLss},
    {BinaryNodeType::Leq, VarType::I32, VarType::I32, VarType::I32, ILInst(ILInstOprType::Leq), EvalLeq},
    {BinaryNodeType::Gtr, VarType::I32, VarType::I32, VarType::I32, ILInst(ILInstOprType::Gtr), EvalGtr},
    {BinaryNodeType::Geq, VarType::I32, VarType::I32, VarType::I32, ILInst(ILInstOprType::Geq), EvalGeq},

    // 浮点数类型二元操作
    {BinaryNodeType::Add, VarType::F32, VarType::F32, VarType::F32, ILInst(ILInstOprType::FAdd), EvalFAdd},
    {BinaryNodeType::Sub, VarType::F32, VarType::F32, VarType::F32, ILInst(ILInstOprType::FSub), EvalFSub},
    {BinaryNodeType::Mul, VarType::F32, VarType::F32, VarType::F32, ILInst(ILInstOprType::FMul), EvalFMul},
    {BinaryNodeType::Div, VarType::F32, VarType::F32, VarType::F32, ILInst(ILInstOprType::FDiv), EvalFDiv},
    {BinaryNodeType::Eql, VarType::F32, VarType::F32, VarType::I32, ILInst(ILInstOprType::FEql), EvalFEql},
    {BinaryNodeType::Neq, VarType::F32, VarType::F32, VarType::I32, ILInst(ILInstOprType::FNeq), EvalFNeq},
    {BinaryNodeType::Lss, VarType::F32, VarType::F32, VarType::I32, ILInst(ILInstOprType::FLss), EvalFLss},
    {BinaryNodeType::Leq, VarType::F32, VarType::F32, VarType::I32, ILInst(ILInstOprType::FLeq), EvalFLeq},
    {BinaryNodeType::Gtr, VarType::F32, VarType::F32, VarType::I32, ILInst(ILInstOprType::FGtr), EvalFGtr},
    {BinaryNodeType::Geq, VarType::F32, VarType::F32, VarType::I32, ILInst(ILInstOprType::FGeq), EvalFGeq},
};