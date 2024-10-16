#pragma once

inline bool IsSpace(char ch)
{
    return ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r';
}

inline bool IsDigit(char ch)
{
    return ch >= '0' && ch <= '9';
}

inline bool IsLetter(char ch)
{
    return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z');
}

inline bool IsLetterOrDigit(char ch)
{
    return IsLetter(ch) || IsDigit(ch);
}