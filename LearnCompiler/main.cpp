#include <iostream>
#include <chrono>

import FileUtils;
import Tokenizer;
import Parser;
import ILInterpreter;

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        std::cout << "用法：" << argv[0] << " 文件名\n";
        return 0;
    }
    const char* filename = argv[1];
    auto text = ReadFile(filename);

    if (!text)
    {
        std::cout << "文件" << filename << "无法打开\n";
        return 0;
    }

    Tokenizer tokenizer(filename, std::move(*text));
    Parser parser(tokenizer);


    try
    {
        parser.Parse();

        std::cout << "=====中间指令=====" << '\n';
        auto& code = parser.code;
        for (size_t i = 0; i < code.size(); i++)
        {
            std::cout << i << ": " << code[i].ToString() << '\n';
        }
        std::cout << '\n';

        std::cout << "=====开始执行=====" << '\n';

        ILInterpreter interpreter;
        auto start = std::chrono::steady_clock::now();
        interpreter.Interpret(code);
        auto end = std::chrono::steady_clock::now();

        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        if (ms >= 10000)
        {
            std::cout << "程序耗时: " << ms / 1000.0 << "s\n";
        }
        else
        {
            std::cout << "程序耗时: " << ms << "ms\n";
        }
    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << '\n';
    }
    return 0;
}