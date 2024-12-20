export module CharUtils;

export bool IsSpace(const char ch)
{
    return ch == ' ' || ch == '\t' || ch == '\n';
}

export bool IsNewline(const char ch)
{
    return ch == '\n';
}

export bool IsDigit(const char ch)
{
    return ch >= '0' && ch <= '9';
}

export bool IsLetter(const char ch)
{
    return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z');
}

export bool IsLetterOrDigit(const char ch)
{
    return IsLetter(ch) || IsDigit(ch);
}

export bool IsPlusOrMinus(const char ch)
{
    return ch == '+' || ch == '-';
}

export bool IsNumeric(const char ch)
{
    return IsDigit(ch) || ch == '.';
}