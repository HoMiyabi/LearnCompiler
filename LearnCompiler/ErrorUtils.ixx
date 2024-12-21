module;
#include <string>
#include <stdexcept>

export module ErrorUtils;
import FileLocation;

// 词法错误
export void ThrowLexical(const std::string_view filePath, FileLocation loc, const std::string_view message)
{
	std::string s(filePath);
	s += loc.SimpleToString();
	s += ": 词法错误: ";
	s += message;
	throw std::runtime_error(s);
}

// 语法错误
export std::string ThrowSyntax(const std::string_view filePath, FileLocation loc, const std::string_view message)
{
	std::string s(filePath);
	s += loc.SimpleToString();
	s += ": 语法错误: ";
	s += message;
	throw std::runtime_error(s);
}

// 语义错误
export std::string ThrowSemantic(const std::string_view filePath, FileLocation loc, const std::string_view message)
{
	std::string s(filePath);
	s += loc.SimpleToString();
	s += ": 语义错误: ";
	s += message;
	throw std::runtime_error(s);
}