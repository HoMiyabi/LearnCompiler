﻿#pragma once

inline bool IsSpace(const char ch)
{
    return ch == ' ' || ch == '\t' || ch == '\n';
}

inline bool IsNewline(const char ch)
{
    return ch == '\n';
}

inline bool IsDigit(const char ch)
{
    return ch >= '0' && ch <= '9';
}

inline bool IsLetter(const char ch)
{
    return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z');
}

inline bool IsLetterOrDigit(const char ch)
{
    return IsLetter(ch) || IsDigit(ch);
}

inline bool IsPlusOrMinus(const char ch)
{
    return ch == '+' || ch == '-';
}

inline bool IsNumeric(const char ch)
{
    return IsDigit(ch) || ch == '.';
}