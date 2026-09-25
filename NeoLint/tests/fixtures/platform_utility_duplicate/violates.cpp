#include <string>

std::wstring ToWideString(const std::string&)
{
    return {};
}

std::string WideStringToString(const wchar_t* value)
{
    return value ? std::string{} : std::string{};
}

std::string TrimLocal(std::string value)
{
    return value;
}

static int ParseSomething(int value)
{
    return value;
}
