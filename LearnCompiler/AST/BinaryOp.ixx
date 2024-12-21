module;
#include <unordered_set>
#include <variant>

export module BinaryOp;
import ValueNode;
import BinaryNodeType;
import VarInfo;
import ILInst;
import ILInstOprType;

export class BuiltInBinaryOp
{
public:
    BinaryNodeType op;
    VarType t1;
    VarType t2;

    VarType ret = VarType::I32;
    ILInst inst{};
    ValueNode* (*eval)(ValueNode*, ValueNode*) = nullptr;

    BuiltInBinaryOp(BinaryNodeType op, VarType t1, VarType t2):
        op(op), t1(t1), t2(t2)
    {
    }

    BuiltInBinaryOp(BinaryNodeType op, VarType t1, VarType t2, VarType ret, ILInst inst,
                    ValueNode* (*eval)(ValueNode*, ValueNode*)):
        op(op), t1(t1), t2(t2), ret(ret), inst(inst), eval(eval)
    {
    }

    bool operator==(const BuiltInBinaryOp& other) const
    {
        return op == other.op && t1 == other.t1 && t2 == other.t2;
    }
};

export struct BuiltInBinaryOpHasher
{
    std::size_t operator()(const BuiltInBinaryOp& op) const
    {
        std::hash<int32_t> h;
        return
            h(static_cast<int32_t>(op.op)) ^
            h(static_cast<int32_t>(op.t1)) ^
            h(static_cast<int32_t>(op.t2));
    }
};

// i32
ValueNode* EvalAdd(ValueNode* node1, ValueNode* node2)
{
    int32_t value = std::get<int32_t>(node1->value) + std::get<int32_t>(node2->value);
    return new ValueNode(VarType::I32, value);
}

ValueNode* EvalSub(ValueNode* node1, ValueNode* node2)
{
    int32_t value = std::get<int32_t>(node1->value) - std::get<int32_t>(node2->value);
    return new ValueNode(VarType::I32, value);
}

ValueNode* EvalMul(ValueNode* node1, ValueNode* node2)
{
    int32_t value = std::get<int32_t>(node1->value) * std::get<int32_t>(node2->value);
    return new ValueNode(VarType::I32, value);
}

ValueNode* EvalDiv(ValueNode* node1, ValueNode* node2)
{
    int32_t value = std::get<int32_t>(node1->value) / std::get<int32_t>(node2->value);
    return new ValueNode(VarType::I32, value);
}

ValueNode* EvalEql(ValueNode* node1, ValueNode* node2)
{
    int32_t value = std::get<int32_t>(node1->value) == std::get<int32_t>(node2->value);
    return new ValueNode(VarType::I32, value);
}

ValueNode* EvalNeq(ValueNode* node1, ValueNode* node2)
{
    int32_t value = std::get<int32_t>(node1->value) != std::get<int32_t>(node2->value);
    return new ValueNode(VarType::I32, value);
}

ValueNode* EvalLss(ValueNode* node1, ValueNode* node2)
{
    int32_t value = std::get<int32_t>(node1->value) < std::get<int32_t>(node2->value);
    return new ValueNode(VarType::I32, value);
}

ValueNode* EvalLeq(ValueNode* node1, ValueNode* node2)
{
    int32_t value = std::get<int32_t>(node1->value) <= std::get<int32_t>(node2->value);
    return new ValueNode(VarType::I32, value);
}

ValueNode* EvalGtr(ValueNode* node1, ValueNode* node2)
{
    int32_t value = std::get<int32_t>(node1->value) > std::get<int32_t>(node2->value);
    return new ValueNode(VarType::I32, value);
}

ValueNode* EvalGeq(ValueNode* node1, ValueNode* node2)
{
    int32_t value = std::get<int32_t>(node1->value) >= std::get<int32_t>(node2->value);
    return new ValueNode(VarType::I32, value);
}

// f32
ValueNode* EvalFAdd(ValueNode* node1, ValueNode* node2)
{
    float value = std::get<float>(node1->value) + std::get<float>(node2->value);
    return new ValueNode(VarType::F32, value);
}

ValueNode* EvalFSub(ValueNode* node1, ValueNode* node2)
{
    float value = std::get<float>(node1->value) - std::get<float>(node2->value);
    return new ValueNode(VarType::F32, value);
}

ValueNode* EvalFMul(ValueNode* node1, ValueNode* node2)
{
    float value = std::get<float>(node1->value) * std::get<float>(node2->value);
    return new ValueNode(VarType::F32, value);
}

ValueNode* EvalFDiv(ValueNode* node1, ValueNode* node2)
{
    float value = std::get<float>(node1->value) / std::get<float>(node2->value);
    return new ValueNode(VarType::F32, value);
}

ValueNode* EvalFEql(ValueNode* node1, ValueNode* node2)
{
    int32_t value = std::get<float>(node1->value) == std::get<float>(node2->value);
    return new ValueNode(VarType::I32, value);
}

ValueNode* EvalFNeq(ValueNode* node1, ValueNode* node2)
{
    int32_t value = std::get<float>(node1->value) != std::get<float>(node2->value);
    return new ValueNode(VarType::I32, value);
}

ValueNode* EvalFLss(ValueNode* node1, ValueNode* node2)
{
    int32_t value = std::get<float>(node1->value) < std::get<float>(node2->value);
    return new ValueNode(VarType::I32, value);
}

ValueNode* EvalFLeq(ValueNode* node1, ValueNode* node2)
{
    int32_t value = std::get<float>(node1->value) <= std::get<float>(node2->value);
    return new ValueNode(VarType::I32, value);
}

ValueNode* EvalFGtr(ValueNode* node1, ValueNode* node2)
{
    int32_t value = std::get<float>(node1->value) > std::get<float>(node2->value);
    return new ValueNode(VarType::I32, value);
}

ValueNode* EvalFGeq(ValueNode* node1, ValueNode* node2)
{
    int32_t value = std::get<float>(node1->value) >= std::get<float>(node2->value);
    return new ValueNode(VarType::I32, value);
}

export std::unordered_set<BuiltInBinaryOp, BuiltInBinaryOpHasher> builtInBinaryOps
{
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