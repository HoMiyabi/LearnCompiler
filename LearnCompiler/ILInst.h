#pragma once
#include "ILInstType.h"
#include "include/magic_enum/magic_enum.hpp"

class ILInst
{
public:
    ILInstType F; // 伪操作码
    int32_t L; // 调用层和说明层的层差值
    int32_t A; // 位移量（相对地址）

    ILInst(const ILInstType F, const int L, const int A) : F(F), L(L), A(A) {}

    [[nodiscard]]
    std::string ToString() const
    {
        return std::string(magic_enum::enum_name(F)) + " " + std::to_string(L) + " " + std::to_string(A);
    }
};
