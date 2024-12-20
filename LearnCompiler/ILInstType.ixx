export module ILInstType;

export enum class ILInstType
{
    LIT, // Load Immediate 立即数入栈
    OPR, // Operation
    LOD, // Load 变量入栈
    STO, // Store 栈顶元素出栈，存入变量
    CAL, // Call
    INT, // Increase stack Top
    JMP, // Jump
    JPC, // Jump on Condition
    RED, // Read
    WRT, // Write
    FRED,
    FWRT
};

export const char* to_string(ILInstType e)
{
    switch (e)
    {
    case ILInstType::LIT: return "LIT";
    case ILInstType::OPR: return "OPR";
    case ILInstType::LOD: return "LOD";
    case ILInstType::STO: return "STO";
    case ILInstType::CAL: return "CAL";
    case ILInstType::INT: return "INT";
    case ILInstType::JMP: return "JMP";
    case ILInstType::JPC: return "JPC";
    case ILInstType::RED: return "RED";
    case ILInstType::WRT: return "WRT";
    case ILInstType::FRED: return "FRED";
    case ILInstType::FWRT: return "FWRT";
    default: return "unknown";
    }
}
