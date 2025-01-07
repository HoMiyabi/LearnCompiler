module;
#include <optional>
#include <stdexcept>
#include <string>

// 导出模块定义，用于其他模块导入
export module Tokenizer;
import FileLocation;
import Token;
import CharUtils;
import TokenKind;
import ErrorUtils;

// Tokenizer类负责词法分析，将输入文本分解成Token
export class Tokenizer
{
public:
    std::string filePath; // 文件路径，用于错误报告
    std::string text; // 输入文本
    std::string::iterator iterator; // 当前解析位置的迭代器
    FileLocation fileLocation; // 当前文件位置，用于跟踪行号和列号

public:
    // 构造函数，初始化Tokenizer
    explicit Tokenizer(std::string filePath, std::string text):
    filePath(std::move(filePath)), text(std::move(text))
    {
        iterator = this->text.begin();
        ProcessBOM(); // 处理文件开头的BOM
    }

    // 获取下一个Token
    std::optional<Token> GetToken()
    {
        SkipSpaceComment(); // 跳过空格和注释

        if (IsEnd())
        {
            return std::nullopt; // 如果到达文本末尾，返回空
        }

        // 根据当前字符的类型，调用不同的处理函数
        if (IsLetter(Current()))
        {
            return HandleKeywordOrIdentifier();
        }

        if (IsDigit(Current()))
        {
            return HandleLiteral();
        }

        if (IsPlusOrMinus(Current()))
        {
            auto next = iterator + 1;
            if (next != text.end() && IsDigit(*next))
            {
                return HandleLiteral();
            }
        }

        if (startingPunctuators.contains(Current()))
        {
            return HandlePunctuator();
        }

        // 如果当前字符无法识别，抛出异常
        ThrowLexical(filePath, fileLocation, "未定义的字符" + Current());
    }

private:
    // 跳过空格和注释
    void SkipSpaceComment()
    {
        while (true)
        {
            if (IsEnd())
            {
                return;
            }
            if (Current() == '#')
            {
                MoveNext();
                while (true)
                {
                    if (IsEnd())
                    {
                        return;
                    }
                    if (IsNewline(Current()))
                    {
                        MoveNext();
                        break;
                    }
                    MoveNext();
                }
            }
            if (!IsSpace(Current()))
            {
                return;
            }
            MoveNext();
        }
    }

    // 处理文件开头的BOM
    void ProcessBOM()
    {
        if (text.size() >= 3 && text[0] == '\xEF' && text[1] == '\xBB' && text[2] == '\xBF')
        {
            iterator = text.begin() + 3;
        }
    }

    // 获取当前字符
    char Current() const
    {
        return *iterator;
    }

    // 移动到下一个字符
    void MoveNext()
    {
        if (IsNewline(*iterator))
        {
            fileLocation.ToNewline();
        }
        else
        {
            ++fileLocation.column;
        }
        ++iterator;
    }

    // 检查是否到达文本末尾
    bool IsEnd() const
    {
        return iterator == text.end();
    }

    // 处理关键字或标识符
    Token HandleKeywordOrIdentifier()
    {
        const auto location = fileLocation;

        std::string tokenStr;
        do
        {
            tokenStr.push_back(Current());
            MoveNext();
        } while (!IsEnd() && IsLetterOrDigit(Current()));

        if (const auto it = keywordSpellingToTokenKind.find(tokenStr); it != keywordSpellingToTokenKind.end())
        {
            return {location, it->second, std::move(tokenStr), filePath};
        }

        return {location, TokenKind::Identifier, std::move(tokenStr), filePath};
    }

    // 处理字面量
    Token HandleLiteral()
    {
        const auto location = fileLocation;

        bool bFloat = false;

        std::string tokenStr;
        do
        {
            if (Current() == '.')
            {
                bFloat = true;
            }
            tokenStr.push_back(Current());
            MoveNext();
        } while (!IsEnd() && IsNumeric(Current()));

        try
        {
            if (bFloat)
            {
                float f = std::stof(tokenStr);
                return {location, TokenKind::Float32, f, std::move(tokenStr), filePath};
            }
            int i = std::stoi(tokenStr);
            return {location, TokenKind::Int32, i, std::move(tokenStr), filePath};
        }
        catch (std::invalid_argument&)
        {
            ThrowLexical(filePath, location, "字面量" + tokenStr + "无法被解析");
        }
        catch (std::out_of_range&)
        {
            ThrowLexical(filePath, location, "字面量" + tokenStr + "超出范围");
        }
    }

    // 处理标点符
    Token HandlePunctuator()
    {
        const auto location = fileLocation;

        if (const auto it = singlePunctuatorSpellingToTokenKind.find(Current());
            it != singlePunctuatorSpellingToTokenKind.end())
        {
            MoveNext();
            return {location, it->second, std::string{it->first}, filePath};
        }

        if (Current() == '<')
        {
            MoveNext();
            if (!IsEnd() && Current() == '>')
            {
                MoveNext();
                return {location, TokenKind::LessGreater, "<>", filePath};
            }
            if (!IsEnd() && Current() == '=')
            {
                MoveNext();
                return {location, TokenKind::LessEqual, "<=", filePath};
            }
            return {location, TokenKind::Less, "<", filePath};
        }

        if (Current() == '>')
        {
            MoveNext();
            if (!IsEnd() && Current() == '=')
            {
                MoveNext();
                return {location, TokenKind::GreaterEqual, ">=", filePath};
            }
            return {location, TokenKind::Greater, ">", filePath};
        }

        if (Current() == ':')
        {
            MoveNext();
            if (!IsEnd() && Current() == '=')
            {
                MoveNext();
                return {location, TokenKind::ColonEqual, ":=", filePath};
            }
            return {location, TokenKind::Colon, ":", filePath};
        }

        ThrowLexical(filePath, location, "未定义的标点符" + Current());
    }
};