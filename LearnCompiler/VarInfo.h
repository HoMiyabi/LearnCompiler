#pragma once
#include <string>

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
    int32_t value = 0;

    explicit VarInfo(VarAttribute attribute, VarType type, std::string name, int32_t runtimeAddress):
    attribute(attribute),
    type(type),
    name(std::move(name)),
    runtimeAddress(runtimeAddress)
    {
    }

    VarInfo() = default;
};