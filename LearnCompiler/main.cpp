#include <iostream>

#include "Token.h"
#include "Tokenizer.h"

int main(int argc, char* argv[])
{
    Tokenizer tker("123 abc");
    Token tk{};
    while (tker.GetToken(tk))
    {
        std::cout << tk.ToString() << '\n';
    }
    return 0;
}
