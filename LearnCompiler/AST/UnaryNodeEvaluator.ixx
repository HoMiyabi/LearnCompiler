module;
#include <unordered_set>
#include <variant>

export module UnaryNodeEvaluator;
import ValueNode;
import UnaryNodeType;
import VarInfo;
import ILInst;
import ILInstOprType;

export struct UnaryNodeEvaluator
{
    UnaryNodeType type;
    VarType t;
    VarType ret = VarType::I32;

    ILInst inst{};
    ValueNode* (*eval)(ValueNode*) = nullptr;

    UnaryNodeEvaluator(UnaryNodeType type, VarType t):
    type(type), t(t)
    {
    }

    UnaryNodeEvaluator(UnaryNodeType type, VarType t, VarType ret, ILInst inst, ValueNode* (*eval)(ValueNode*)):
    type(type), t(t), ret(ret), inst(inst), eval(eval)
    {
    }

    bool operator==(const UnaryNodeEvaluator& rhs) const
    {
        return type == rhs.type && t == rhs.t;
    }
};

ValueNode* EvalNeg(ValueNode* node)
{
    return new ValueNode(VarType::I32, -std::get<int32_t>(node->value));
}

ValueNode* EvalFNeg(ValueNode* node)
{
    return new ValueNode(VarType::F32, -std::get<float>(node->value));
}

ValueNode* EvalOdd(ValueNode* node)
{
    return new ValueNode(VarType::I32, std::get<int32_t>(node->value) % 2);
}

ValueNode* EvalToI32(ValueNode* node)
{
    return new ValueNode(VarType::I32, static_cast<int32_t>(std::get<float>(node->value)));
}

ValueNode* EvalToF32(ValueNode* node)
{
    return new ValueNode(VarType::F32, static_cast<float>(std::get<int32_t>(node->value)));
}

struct UnaryNodeEvaluatorHasher
{
    std::size_t operator()(const UnaryNodeEvaluator& op) const
    {
        std::hash<int32_t> h;
        return
            h(static_cast<int32_t>(op.type)) ^
            h(static_cast<int32_t>(op.t));
    }
};

export std::unordered_set<UnaryNodeEvaluator, UnaryNodeEvaluatorHasher> unaryNodeEvaluators
{
    {UnaryNodeType::Neg, VarType::I32, VarType::I32, ILInst(ILInstOprType::Neg), EvalNeg},
    {UnaryNodeType::Neg, VarType::F32, VarType::F32, ILInst(ILInstOprType::FNeg), EvalFNeg},
    {UnaryNodeType::Odd, VarType::I32, VarType::I32, ILInst(ILInstOprType::Odd), EvalOdd},
    {UnaryNodeType::ToI32, VarType::F32, VarType::I32, ILInst(ILInstOprType::FToI), EvalToI32},
    {UnaryNodeType::ToF32, VarType::I32, VarType::F32, ILInst(ILInstOprType::IToF), EvalToF32},
};