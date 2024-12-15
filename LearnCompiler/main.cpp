#include <iostream>

#include "FileUtils.h"
#include "ILInterpreter.h"
#include "Parser.h"
#include "Token.h"
#include "Tokenizer.h"

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        std::cout << "请输入文件名\n";
        return 0;
    }
    std::string fileName(argv[1]);
    auto text = ReadFile(fileName);

    Tokenizer tokenizer(std::move(text));
    Parser parser(tokenizer);

    // parser.Parse();
    try
    {
        parser.Parse();
    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << '\n';
        return 0;
    }

    auto& code = parser.code;
    for (size_t i = 0; i < code.size(); i++)
    {
        std::cout << i << ": " << code[i].ToString() << '\n';
    }
    ILInterpreter().Interpret(code);

    return 0;
}