module;
#include <string_view>
#include <unordered_set>
#include <unordered_map>

export module TokenKind;

export enum class TokenKind
{
    Identifier,
    // 字面量
    Int32,
    Float32,

    // 关键字
    Begin,
    End,
    If,
    Then,
    Else,
    While,
    Do,
    Var,
    Procedure,
    Const,
    Odd,
    Read,
    Write,
    Program,
    Return,

    // 类型关键字
    I32,
    F32,

    // 标点符
    //  运算符
    Plus,
    Minus,
    Star,
    Slash,

    //  比较符
    Equal,
    LessGreater,
    Less,
    LessEqual,
    Greater,
    GreaterEqual,

    //  括号
    LParen,
    RParen,

    //  分隔符
    Semi, // ;
    Comma, // ,
    Colon, // :
    ColonEqual, // :=
};

export const char* to_string(TokenKind e)
{
    switch (e)
    {
    case TokenKind::Identifier: return "Identifier";
    case TokenKind::Int32: return "Int32";
    case TokenKind::Float32: return "Float32";
    case TokenKind::Begin: return "Begin";
    case TokenKind::End: return "End";
    case TokenKind::If: return "If";
    case TokenKind::Then: return "Then";
    case TokenKind::Else: return "Else";
    case TokenKind::While: return "While";
    case TokenKind::Do: return "Do";
    case TokenKind::Var: return "Var";
    case TokenKind::Procedure: return "Procedure";
    case TokenKind::Const: return "Const";
    case TokenKind::Odd: return "Odd";
    case TokenKind::Read: return "Read";
    case TokenKind::Write: return "Write";
    case TokenKind::Program: return "Program";
    case TokenKind::Return: return "Return";
    case TokenKind::I32: return "I32";
    case TokenKind::F32: return "F32";
    case TokenKind::Plus: return "Plus";
    case TokenKind::Minus: return "Minus";
    case TokenKind::Star: return "Star";
    case TokenKind::Slash: return "Slash";
    case TokenKind::Equal: return "Equal";
    case TokenKind::LessGreater: return "LessGreater";
    case TokenKind::Less: return "Less";
    case TokenKind::LessEqual: return "LessEqual";
    case TokenKind::Greater: return "Greater";
    case TokenKind::GreaterEqual: return "GreaterEqual";
    case TokenKind::LParen: return "LParen";
    case TokenKind::RParen: return "RParen";
    case TokenKind::Semi: return "Semi";
    case TokenKind::Comma: return "Comma";
    case TokenKind::Colon: return "Colon";
    case TokenKind::ColonEqual: return "ColonEqual";
    default: return "unknown";
    }
}

export const std::unordered_map<std::string_view, TokenKind> keywordSpellingToTokenKind
{
    {"begin", TokenKind::Begin},
    {"end", TokenKind::End},
    {"if", TokenKind::If},
    {"then", TokenKind::Then},
    {"else", TokenKind::Else},
    {"while", TokenKind::While},
    {"do", TokenKind::Do},
    {"var", TokenKind::Var},
    {"procedure", TokenKind::Procedure},
    {"const", TokenKind::Const},
    {"odd", TokenKind::Odd},
    {"read", TokenKind::Read},
    {"write", TokenKind::Write},
    {"program", TokenKind::Program},
    {"return", TokenKind::Return},
    {"i32", TokenKind::I32},
    {"f32", TokenKind::F32},
};

// 同时表示没有他们开头的其他长标点符
export const std::unordered_map<char, TokenKind> singlePunctuatorSpellingToTokenKind
{
    {'+', TokenKind::Plus},
    {'-', TokenKind::Minus},
    {'*', TokenKind::Star},
    {'/', TokenKind::Slash},

    {'=', TokenKind::Equal},

    {'(', TokenKind::LParen},
    {')', TokenKind::RParen},

    {';', TokenKind::Semi},
    {',', TokenKind::Comma},
};

export const std::unordered_map<std::string_view, TokenKind> punctuatorSpellingToTokenKind
{
    {"+", TokenKind::Plus},
    {"-", TokenKind::Minus},
    {"*", TokenKind::Star},
    {"/", TokenKind::Slash},

    {"=", TokenKind::Equal},
    {"<>", TokenKind::LessGreater},
    {"<", TokenKind::Less},
    {"<=", TokenKind::LessEqual},
    {">", TokenKind::Greater},
    {">=", TokenKind::GreaterEqual},

    {"(", TokenKind::LParen},
    {")", TokenKind::RParen},

    {";", TokenKind::Semi},
    {",", TokenKind::Comma},
    {":", TokenKind::Colon},
    {":=", TokenKind::ColonEqual},
};

export const std::unordered_set<char> startingPunctuators
{
    '+',
    '-',
    '*',
    '/',
    '=',
    '<',
    '>',
    '(',
    ')',
    ';',
    ',',
    ':',
};