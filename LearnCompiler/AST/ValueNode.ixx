module;
#include <variant>
#include <vector>

export module ValueNode;
import ASTNode;
import ILInst;
import VarInfo;

export struct ValueNode : ASTNode
{
    std::variant<int32_t, float> value;

    explicit ValueNode(VarType varType, std::variant<int32_t, float> value):
    ASTNode(varType), value(value)
    {
    }

    void GenerateCode(std::vector<ILInst>& code) override
    {
        if (varType == VarType::I32)
        {
            code.push_back(ILInst::LIT(std::get<int32_t>(value)));
        }
        else if (varType == VarType::F32)
        {
            code.push_back(ILInst::LIT(std::get<float>(value)));
        }
    };
};
