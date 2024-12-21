module;
#include <string>
#include <variant>

export module Token;
import TokenKind;
import FileLocation;

export class Token
{
public:
    TokenKind kind;
    std::variant<int32_t, float> value;
    FileLocation fileLocation;
    std::string rawText;
    std::string_view filePath;

    Token() = default;

    Token(const FileLocation fileLocation, const TokenKind kind, std::string rawText, const std::string_view filePath):
    kind(kind), fileLocation(fileLocation), rawText(std::move(rawText)), filePath(filePath)
    {
    }

    Token(const FileLocation fileLocation, const TokenKind kind, std::variant<int, float> value,
        std::string rawText, const std::string_view filePath):
    kind(kind), value(std::move(value)), fileLocation(fileLocation), rawText(std::move(rawText)), filePath(filePath)
    {
    }

    [[nodiscard]]
    int32_t Int32() const
    {
        return std::get<int32_t>(value);
    }

    [[nodiscard]]
    float Float32() const
    {
        return std::get<float>(value);
    }

    std::string ToString() const
    {
        std::string text;
        text += to_string(kind);
        if (kind == TokenKind::Int32)
        {
            text += ' ';
            text += std::to_string(std::get<int32_t>(value));
        }
        else if (kind == TokenKind::Identifier)
        {
            text += ' ';
            text += rawText;
        }
        return text;
    }
};
