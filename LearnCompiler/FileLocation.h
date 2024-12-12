#pragma once
#include <string>

class FileLocation
{
public:
    int row = 1;
    int column = 1;

    [[nodiscard]] std::string ToString() const
    {
        return std::to_string(row) + "行" + std::to_string(column) + "列";
    }

    void ToNewline()
    {
        ++row;
        column = 1;
    }
};
