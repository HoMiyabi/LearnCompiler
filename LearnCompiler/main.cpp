#include <iostream>

#include "FileUtils.h"
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
    // std::cout << text << '\n';

    Tokenizer tokenizer(std::move(text));
    Token token;
    while (tokenizer.GetToken(token))
    {
        std::cout << token.ToString() << '\n';
    }
    if (!tokenizer.message.empty())
    {
        std::cout << tokenizer.message << '\n';
    }

    return 0;
}