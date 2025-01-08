export module UnaryNodeType;

// 导出一个枚举类UnaryNodeType，表示一元运算的类型
export enum class UnaryNodeType
{
    // 表示负值运算
    Neg,
    // 表示奇数运算
    Odd,
    // 表示转换为32位整数运算
    ToI32,
    // 表示转换为32位浮点数运算
    ToF32,
};

// 导出一个函数to_string，用于将UnaryNodeType枚举类的值转换为对应的字符串表示
export const char* to_string(UnaryNodeType e)
{
    // 根据传入的UnaryNodeType值，返回相应的字符串
    switch (e)
    {
    case UnaryNodeType::Neg: return "Neg";
    case UnaryNodeType::Odd: return "Odd";
    case UnaryNodeType::ToI32: return "ToI32";
    case UnaryNodeType::ToF32: return "ToF32";
    // 如果值不属于已定义的枚举成员，则返回"unknown"
    default: return "unknown";
    }
}