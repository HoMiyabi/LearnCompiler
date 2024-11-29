#pragma once
#include <fstream>
#include <sstream>
#include <string>

inline std::string ReadFile(const std::string& fileName)
{
    const std::ifstream ifs{fileName};
    std::ostringstream ss;
    ss << ifs.rdbuf();
    return ss.str();
}