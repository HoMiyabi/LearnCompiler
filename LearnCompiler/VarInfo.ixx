module;
#include <string>
#include <variant>

export module VarInfo;

export enum class VarType
{
    Void,
    I32,
    F32,
};

export const char* to_string(VarType e)
{
    switch (e)
    {
    case VarType::Void: return "Void";
    case VarType::I32: return "I32";
    case VarType::F32: return "F32";
    default: return "unknown";
    }
}

export class VarInfo
{
public:
    bool bConst;
    VarType type;
    std::string name;
    int32_t runtimeAddress;
    std::variant<int32_t, float> value;

    explicit VarInfo(bool bConst, const VarType type, std::string name, const int32_t runtimeAddress):
    bConst(bConst),
    type(type),
    name(std::move(name)),
    runtimeAddress(runtimeAddress)
    {
    }

    VarInfo() = default;
};