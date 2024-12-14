#pragma once
#include <iostream>
#include <stdexcept>
#include <vector>
#include "ILInst.h"
#include "ILInstOprType.h"

class ILInterpreter
{
private:
    ILInst I {ILInstType::LIT, 0, 0}; // 指令寄存器I: 存放当前要执行的代码
    // int T = 0; // 栈顶指示器寄存器T：指向数据栈STACK的栈顶的下一个元素
    int32_t rbp = 0; // 基地址寄存器B：存放当前运行过程的数据区在STACK中的起始地址
    int32_t rip = 0; // 程序地址寄存器P：存放下一条要执行的指令地址
    std::vector<int32_t> stk;

public:
    ILInterpreter()
    {
        stk.reserve(1024);
    }

    void Interpret(const std::vector<ILInst>& code)
    {
        stk.clear();
        // T = 0;
        rbp = 0;
        rip = 0;
        while (rip != code.size())
        {
            Fetch(code);
            switch (I.F)
            {
            case ILInstType::LIT:
                {
                    stk.push_back(I.A);
                    break;
                }
            case ILInstType::OPR:
                {
                    HandleOpr();
                    break;
                }
            case ILInstType::LOD:
                {
                    stk.push_back(stk[GetSL() + I.A]);
                    break;
                }
            case ILInstType::STO:
                {
                    stk[GetSL() + I.A] = stk.back();
                    stk.pop_back();
                    break;
                }
            case ILInstType::CAL:
                {
                    stk.push_back(GetSL());
                    stk.push_back(rip);
                    stk.push_back(rbp);
                    rbp = stk.size();
                    rip = I.A;
                    break;
                }
            case ILInstType::INT:
                {
                    stk.resize(stk.size() + I.A);
                    break;
                }
            case ILInstType::JMP:
                {
                    rip = I.A;
                    break;
                }
            case ILInstType::JPC:
                {
                    if (stk.back() == 0)
                    {
                        rip = I.A;
                    }
                    stk.pop_back();
                    break;
                }
            case ILInstType::RED:
                {
                    std::cin >> stk[GetSL() + I.A];
                    break;
                }
            case ILInstType::WRT:
                {
                    std::cout << stk.back() << '\n';
                    stk.pop_back();
                    break;
                }
            }
        }
    }

private:
    int32_t GetSL() const
    {
        int32_t p = rbp;
        int l = I.L;
        while (l > 0)
        {
            p = stk[p - 3];
            l--;
        }
        return p;
    }

    void HandleOpr()
    {
        switch (static_cast<ILInstOprType>(I.A))
        {
        case ILInstOprType::Ret:
            {
                rbp = stk.back();
                stk.pop_back();
                rip = stk.back();
                stk.pop_back();
                stk.pop_back();
                break;
            }
        case ILInstOprType::Neg:
            {
                stk.back() = -stk.back();
                break;
            }
        case ILInstOprType::Odd:
            {
                stk.back() = stk.back() % 2 == 1;
                break;
            }
        case ILInstOprType::Add:
            {
                stk[stk.size() - 2] += stk.back();
                stk.pop_back();
                break;
            }
        case ILInstOprType::Sub:
            {
                stk[stk.size() - 2] -= stk.back();
                stk.pop_back();
                break;
            }
        case ILInstOprType::Mul:
            {
                stk[stk.size() - 2] *= stk.back();
                stk.pop_back();
                break;
            }
        case ILInstOprType::Div:
            {
                stk[stk.size() - 2] /= stk.back();
                stk.pop_back();
                break;
            }
        case ILInstOprType::Eql:
            {
                stk[stk.size() - 2] = (stk[stk.size() - 2] == stk.back());
                stk.pop_back();
                break;
            }
        case ILInstOprType::Neq:
            {
                stk[stk.size() - 2] = (stk[stk.size() - 2] != stk.back());
                stk.pop_back();
                break;
            }
        case ILInstOprType::Lss:
            {
                stk[stk.size() - 2] = (stk[stk.size() - 2] < stk.back());
                stk.pop_back();
                break;
            }
        case ILInstOprType::Leq:
            {
                stk[stk.size() - 2] = (stk[stk.size() - 2] <= stk.back());
                stk.pop_back();
                break;
            }
        case ILInstOprType::Gtr:
            {
                stk[stk.size() - 2] = (stk[stk.size() - 2] > stk.back());
                stk.pop_back();
                break;
            }
        case ILInstOprType::Geq:
            {
                stk[stk.size() - 2] = (stk[stk.size() - 2] >= stk.back());
                stk.pop_back();
                break;
            }
        default:
            {
                throw std::runtime_error("Unknown ILInstOprType");
            }
        }
    }

    void Fetch(const std::vector<ILInst>& code)
    {
        I = code[rip];
        rip++;
    }
};
