#include <fstream>
#include <iostream>
#include <sstream>

#include "Token.h"
#include "Tokenizer.h"

std::string ReadFile(std::string_view fileName)
{
    std::ifstream ifs{fileName.data()};
    std::ostringstream ss;
    ss << ifs.rdbuf();
    return ss.str();
}

int main(int argc, char* argv[])
{
    if (argc != 2) return 0;
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