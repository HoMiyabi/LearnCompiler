module;
#include <fstream>
#include <sstream>
#include <string>

export module FileUtils;

export std::string ReadFile(const std::string& fileName)
{
    const std::ifstream ifs{fileName};
    std::ostringstream ss;
    ss << ifs.rdbuf();
    return ss.str();
}