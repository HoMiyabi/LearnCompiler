#include <iostream>

#include "FileUtils.h"
#include "Parser.h"
#include "Token.h"
#include "Tokenizer.h"

void ProcessBOM(std::string& text)
{
    if (text.size() >= 3 && text[0] == '\xEF' && text[1] == '\xBB' && text[2] == '\xBF')
    {
        text.erase(0, 3);
    }
}

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        std::cout << "请输入文件名\n";
        return 0;
    }
    auto text = ReadFile(argv[1]);
    ProcessBOM(text);

    Tokenizer tokenizer(std::move(text));
    Parser parser(tokenizer);
    parser.Parse();

    return 0;
}