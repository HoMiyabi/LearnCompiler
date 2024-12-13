#pragma once

enum class ILOPCode
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
};