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
    auto text = ReadFile(argv[1]);

    Tokenizer tokenizer(std::move(text));
    Parser parser(tokenizer);
    parser.Parse();

    auto& code = parser.code;
    for (const auto& inst : code)
    {
        std::cout << inst.ToString() << '\n';
    }
    ILInterpreter().Interpret(code);

    return 0;
}