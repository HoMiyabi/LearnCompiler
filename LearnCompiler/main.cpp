#include <iostream>

#include "Token.h"
#include "Tokenizer.h"

int main(int argc, char* argv[])
{
    Tokenizer tker("var a = 1 var bbbbb = 2");
    Token tk;
    while (tker.GetToken(tk))
    {
        std::cout << tk.ToString() << '\n';
    }
    return 0;
}
