export module BinaryNodeType;

export enum class BinaryNodeType
{
    Add,
    Sub,
    Mul,
    Div,
    Eql,
    Neq,
    Lss,
    Leq,
    Gtr,
    Geq,
};

export const char* to_string(BinaryNodeType e)
{
    switch (e)
    {
    case BinaryNodeType::Add: return "Add";
    case BinaryNodeType::Sub: return "Sub";
    case BinaryNodeType::Mul: return "Mul";
    case BinaryNodeType::Div: return "Div";
    case BinaryNodeType::Eql: return "Eql";
    case BinaryNodeType::Neq: return "Neq";
    case BinaryNodeType::Lss: return "Lss";
    case BinaryNodeType::Leq: return "Leq";
    case BinaryNodeType::Gtr: return "Gtr";
    case BinaryNodeType::Geq: return "Geq";
    default: return "unknown";
    }
}
