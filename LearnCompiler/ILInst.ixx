module;
#include <cstdint>
#include <string>

export module ILInst;
import ILInstType;
import ILInstOprType;

export class ILInst
{
public:
    ILInstType F; // 伪操作码
    int32_t L; // 调用层和说明层的层差值
    int32_t A; // 位移量（相对地址）

    ILInst(const ILInstType F, const int32_t L, const int32_t A) : F(F), L(L), A(A) {}

    explicit ILInst(ILInstOprType opr): F(ILInstType::OPR), L(0), A(static_cast<int32_t>(opr))
    {
    }

    ILInst() = default;

    static ILInst LIT(int32_t value)
    {
        return {ILInstType::LIT, 0, value};
    }

    static ILInst LIT(float value)
    {
        return {ILInstType::LIT, 0, *reinterpret_cast<int32_t*>(&value)};
    }

    [[nodiscard]]
    std::string ToString() const
    {
        if (F == ILInstType::OPR)
        {
            return std::string(to_string(F)) + " " + std::to_string(L) + " " +
                std::string(to_string(static_cast<ILInstOprType>(A)));
        }
        if (F == ILInstType::LIT)
        {
            return std::string(to_string(F)) + " " + std::to_string(L) + " i32:" + std::to_string(A)
            + " f32:" + std::to_string(*(float*)(&A));
        }
        return std::string(to_string(F)) + " " + std::to_string(L) + " " + std::to_string(A);
    }
};
