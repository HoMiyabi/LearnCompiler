#include <iostream>
#include <chrono>

import FileUtils;
import Tokenizer;
import Parser;
import ILInterpreter;

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

    ILInterpreter interpreter;
    auto start = std::chrono::steady_clock::now();
    interpreter.Interpret(code);
    auto end = std::chrono::steady_clock::now();
    std::cout << "程序耗时: " <<
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << "ms\n";

    return 0;
}