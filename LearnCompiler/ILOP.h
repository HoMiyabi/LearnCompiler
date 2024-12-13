#pragma once
#include "ILOPCode.h"

class ILOP
{
public:
    ILOPCode F; // 伪操作码
    int32_t L; // 调用层和说明层的层差值
    int32_t A; // 位移量（相对地址）

    ILOP(const ILOPCode F, const int L, const int A) : F(F), L(L), A(A) {}
};
