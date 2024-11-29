#ifndef TOKEN
#define TOKEN(Name)
#endif

#ifndef TOKEN_KEYWORD
#define TOKEN_KEYWORD(Name, Spelling)
#endif

#ifndef TOKEN_PUNCTUATOR
#define TOKEN_PUNCTUATOR(Name, Spelling)
#endif

TOKEN(Identifier)
TOKEN(Int)
TOKEN_KEYWORD(Begin, "begin")
TOKEN_KEYWORD(End, "end")

TOKEN_KEYWORD(If, "if")
TOKEN_KEYWORD(Then, "then")
TOKEN_KEYWORD(Else, "else")

TOKEN_KEYWORD(While, "while")
TOKEN_KEYWORD(Do, "do")

TOKEN_KEYWORD(Var, "var")
TOKEN_KEYWORD(Procedure, "procedure")
TOKEN_KEYWORD(Call, "call")
TOKEN_KEYWORD(Const, "const")
TOKEN_KEYWORD(Odd, "odd")
TOKEN_KEYWORD(Read, "read")
TOKEN_KEYWORD(Write, "write")

TOKEN_PUNCTUATOR(Plus, "+")
TOKEN_PUNCTUATOR(Minus, "-")
TOKEN_PUNCTUATOR(Star, "*")
TOKEN_PUNCTUATOR(Slash, "/")

TOKEN_PUNCTUATOR(Equal, "=")
TOKEN_PUNCTUATOR(Less, "<")
TOKEN_PUNCTUATOR(LessEqual, "<=")
TOKEN_PUNCTUATOR(Greater, ">")
TOKEN_PUNCTUATOR(GreaterEqual, ">=")

TOKEN_PUNCTUATOR(LParen, "(")
TOKEN_PUNCTUATOR(RParen, ")")
TOKEN_PUNCTUATOR(LBrace, "{")
TOKEN_PUNCTUATOR(RBrace, "}")

TOKEN_PUNCTUATOR(Semi, ";")
TOKEN_PUNCTUATOR(Comma, ",")
TOKEN_PUNCTUATOR(ColonEqual, ":=")

#undef TOKEN
#undef TOKEN_KEYWORD
#undef TOKEN_PUNCTUATOR