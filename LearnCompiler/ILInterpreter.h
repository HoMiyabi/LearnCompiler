#pragma once
#include <stack>
#include <vector>
#include "ILOP.h"

class ILInterpreter
{
private:
    ILOP I {ILOPCode::LIT, 0, 0}; // 指令寄存器I: 存放当前要执行的代码
    // int T = 0; // 栈顶指示器寄存器T：指向数据栈STACK的栈顶的下一个元素
    int B = 0; // 基地址寄存器B：存放当前运行过程的数据区在STACK中的起始地址
    int ip = 0; // 程序地址寄存器P：存放下一条要执行的指令地址
    std::vector<int> stk;

public:
    ILInterpreter()
    {
        stk.reserve(1024);
    }

    void Interpret(const std::vector<ILOP>& code)
    {
        if (code.empty()) return;

        stk.clear();
        // T = 0;
        B = 0;
        ip = 0;
        do
        {
            FetchOP(code);
            switch (I.F)
            {
            case ILOPCode::LIT:
                {
                    stk.push_back(I.A);
                    break;
                }
            case ILOPCode::OPR:
                break;
            case ILOPCode::LOD:
                {
                    break;
                }
            case ILOPCode::STO:
                break;
            case ILOPCode::CAL:
                break;
            case ILOPCode::INT:
                break;
            case ILOPCode::JMP:
                {
                    ip = I.A;
                    break;
                }
            case ILOPCode::JPC:
                {
                    if (stk.back() == 0)
                    {
                        ip = I.A;
                    }
                    stk.pop_back();
                    break;
                }
            case ILOPCode::RED:
                break;
            case ILOPCode::WRT:
                break;
            }
        }
        while (ip != code.size());
    }

private:
    void FetchOP(const std::vector<ILOP>& code)
    {
        I = code[ip];
        ip++;
    }
};
