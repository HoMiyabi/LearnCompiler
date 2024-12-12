#pragma once
#include <unordered_set>
#include <unordered_map>

enum class TokenKind
{
    Identifier,
    Int,

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
    Call,
    Const,
    Odd,
    Read,
    Write,
    Program,

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
    LBrace,
    RBrace,

    //  分隔符
    Semi,
    Comma,
    ColonEqual,
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
    {"call", TokenKind::Call},
    {"const", TokenKind::Const},
    {"odd", TokenKind::Odd},
    {"read", TokenKind::Read},
    {"write", TokenKind::Write},
    {"program", TokenKind::Program},
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
    {'{', TokenKind::LBrace},
    {'}', TokenKind::RBrace},

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
    {"{", TokenKind::LBrace},
    {"}", TokenKind::RBrace},

    {";", TokenKind::Semi},
    {",", TokenKind::Comma},
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
    '{',
    '}',
    ';',
    ',',
    ':',
};