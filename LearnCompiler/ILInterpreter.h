#pragma once
#include <vector>
#include "ILInst.h"

class ILInterpreter
{
private:
    ILInst I {ILInstType::LIT, 0, 0}; // 指令寄存器I: 存放当前要执行的代码
    // int T = 0; // 栈顶指示器寄存器T：指向数据栈STACK的栈顶的下一个元素
    int B = 0; // 基地址寄存器B：存放当前运行过程的数据区在STACK中的起始地址
    int ip = 0; // 程序地址寄存器P：存放下一条要执行的指令地址
    std::vector<int> stk;

public:
    ILInterpreter()
    {
        stk.reserve(1024);
    }

    void Interpret(const std::vector<ILInst>& code)
    {
        stk.clear();
        // T = 0;
        B = 0;
        ip = 0;
        while (ip != code.size())
        {
            FetchOP(code);
            switch (I.F)
            {
            case ILInstType::LIT:
                {
                    stk.push_back(I.A);
                    break;
                }
            case ILInstType::OPR:
                break;
            case ILInstType::LOD:
                {
                    break;
                }
            case ILInstType::STO:
                break;
            case ILInstType::CAL:
                break;
            case ILInstType::INT:
                break;
            case ILInstType::JMP:
                {
                    ip = I.A;
                    break;
                }
            case ILInstType::JPC:
                {
                    if (stk.back() == 0)
                    {
                        ip = I.A;
                    }
                    stk.pop_back();
                    break;
                }
            case ILInstType::RED:
                break;
            case ILInstType::WRT:
                break;
            }
        }
    }

private:
    void FetchOP(const std::vector<ILInst>& code)
    {
        I = code[ip];
        ip++;
    }
};
