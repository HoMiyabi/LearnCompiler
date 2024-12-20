export module ILInstOprType;

export enum class ILInstOprType
{
    Ret, // 过程调用结束后,返回调用点并退栈
    Neg, // 栈顶元素取反
    Odd, // 栈顶元素是否为奇数，结果值在栈顶
    Add, // 次栈顶与栈顶相加，退两个栈元素，结果值进栈
    Sub, // 次栈顶减去栈顶，退两个栈元素，结果值进栈
    Mul, // 次栈顶乘以栈顶，退两个栈元素，结果值进栈
    Div, // 次栈顶除以栈顶，退两个栈元素，结果值进栈
    Eql, // 次栈顶是否等于栈顶，退两个栈元素，结果值进栈
    Neq, // 次栈顶是否不等于栈顶，退两个栈元素，结果值进栈
    Lss, // 次栈顶是否小于栈顶，退两个栈元素，结果值进栈
    Leq, // 次栈顶是否小于等于栈顶，退两个栈元素，结果值进栈
    Gtr, // 次栈顶是否大于栈顶，退两个栈元素，结果值进栈
    Geq, // 次栈顶是否大于等于栈顶，退两个栈元素，结果值进栈

    FNeg,
    FAdd,
    FSub,
    FMul,
    FDiv,
    FEql,
    FNeq,
    FLss,
    FLeq,
    FGtr,
    FGeq,
    FToI,
    IToF,
};

export const char* to_string(ILInstOprType e)
{
    switch (e)
    {
    case ILInstOprType::Ret: return "Ret";
    case ILInstOprType::Neg: return "Neg";
    case ILInstOprType::Odd: return "Odd";
    case ILInstOprType::Add: return "Add";
    case ILInstOprType::Sub: return "Sub";
    case ILInstOprType::Mul: return "Mul";
    case ILInstOprType::Div: return "Div";
    case ILInstOprType::Eql: return "Eql";
    case ILInstOprType::Neq: return "Neq";
    case ILInstOprType::Lss: return "Lss";
    case ILInstOprType::Leq: return "Leq";
    case ILInstOprType::Gtr: return "Gtr";
    case ILInstOprType::Geq: return "Geq";
    case ILInstOprType::FNeg: return "FNeg";
    case ILInstOprType::FAdd: return "FAdd";
    case ILInstOprType::FSub: return "FSub";
    case ILInstOprType::FMul: return "FMul";
    case ILInstOprType::FDiv: return "FDiv";
    case ILInstOprType::FEql: return "FEql";
    case ILInstOprType::FNeq: return "FNeq";
    case ILInstOprType::FLss: return "FLss";
    case ILInstOprType::FLeq: return "FLeq";
    case ILInstOprType::FGtr: return "FGtr";
    case ILInstOprType::FGeq: return "FGeq";
    case ILInstOprType::FToI: return "FToI";
    case ILInstOprType::IToF: return "IToF";
    default: return "unknown";
    }
}
