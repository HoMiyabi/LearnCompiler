export module UnaryNodeType;

export enum class UnaryNodeType
{
    Neg,
    Odd,
    ToI32,
    ToF32,
};

export const char* to_string(UnaryNodeType e)
{
    switch (e)
    {
    case UnaryNodeType::Neg: return "Neg";
    case UnaryNodeType::Odd: return "Odd";
    case UnaryNodeType::ToI32: return "ToI32";
    case UnaryNodeType::ToF32: return "ToF32";
    default: return "unknown";
    }
}
