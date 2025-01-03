module;
#include <fstream>
#include <optional>
#include <sstream>
#include <string>

export module FileUtils;

export std::optional<std::string> ReadFile(const char* filename)
{
    const std::ifstream ifs{filename};
    if (!ifs.is_open())
    {
        return std::nullopt;
    }
    std::ostringstream ss;
    ss << ifs.rdbuf();
    return ss.str();
}