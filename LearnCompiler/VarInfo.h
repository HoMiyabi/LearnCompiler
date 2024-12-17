#pragma once
#include <string>
#include <variant>

enum class VarAttribute
{
    Const,
    Var,
};

enum class VarType
{
    I32,
    F32,
};

class VarInfo
{
public:
    VarAttribute attribute;
    VarType type;
    std::string name;
    int32_t runtimeAddress;
    std::variant<int32_t, float> value;

    explicit VarInfo(VarAttribute attribute, VarType type, std::string name, int32_t runtimeAddress):
    attribute(attribute),
    type(type),
    name(std::move(name)),
    runtimeAddress(runtimeAddress)
    {
    }

    VarInfo() = default;
};