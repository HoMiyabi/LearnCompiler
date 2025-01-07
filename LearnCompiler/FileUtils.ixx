module;
#include <fstream>
#include <optional>
#include <sstream>
#include <string>

export module FileUtils;

/**
 * 从文件系统中读取文件内容
 * 
 * @param filename 文件名，指定要读取的文件
 * @return 返回一个可选的字符串，包含文件内容如果文件无法打开，则返回空的std::optional
 */
export std::optional<std::string> ReadFile(const char* filename)
{
    const std::ifstream ifs{filename};
    // 检查文件是否成功打开
    if (!ifs.is_open())
    {
        // 如果文件未成功打开，则返回空的std::optional
        return std::nullopt;
    }
    std::ostringstream ss;
    ss << ifs.rdbuf();
    // 返回字符串流的内容作为文件内容
    return ss.str();
}