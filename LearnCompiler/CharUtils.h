#pragma once

inline bool IsSpace(const char ch)
{
    return ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r';
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