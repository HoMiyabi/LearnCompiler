#pragma once
#include <unordered_set>
#include <unordered_map>

enum class TokenKind
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

inline const std::unordered_map<std::string_view, TokenKind> keywordSpellingToTokenKind
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
inline const std::unordered_map<char, TokenKind> singlePunctuatorSpellingToTokenKind
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

inline const std::unordered_map<std::string_view, TokenKind> punctuatorSpellingToTokenKind
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

inline const std::unordered_set<char> startingPunctuators
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