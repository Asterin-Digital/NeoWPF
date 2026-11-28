#include <NeoWPF/Core/Application/EntryRunner.h>

#include <NeoWPF/Feedback/Result.h>

#include <windows.h>
#include <shellapi.h>

#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace NeoWPF::Platform {
NeoWPF::Result<std::filesystem::path> GetExecutableDirectory();
}

namespace {

NeoWPF::Result<std::vector<std::wstring>> GetArguments()
{
    int argumentCount = 0;
    LPWSTR* rawArguments = ::CommandLineToArgvW(::GetCommandLineW(), &argumentCount);
    if (!rawArguments) {
        return NeoWPF::Fail<std::string>{
            "CommandLineToArgvW failed with error " +
            std::to_string(::GetLastError()) + "."};
    }
    const std::unique_ptr<wchar_t*, decltype(&::LocalFree)> ownedArguments(
        rawArguments, &::LocalFree);

    std::vector<std::wstring> arguments;
    arguments.reserve(static_cast<std::size_t>(argumentCount));
    for (int index = 0; index < argumentCount; ++index) {
        arguments.emplace_back(ownedArguments.get()[index]);
    }
    return NeoWPF::Success<std::vector<std::wstring>>{std::move(arguments)};
}

}

int main()
{
    auto arguments = GetArguments();
    return NeoWPF::Core::EntryRunner::Run(
        std::move(arguments),
        NeoWPF::Success<std::optional<std::string>>{std::nullopt},
        NeoWPF::Platform::GetExecutableDirectory());
}
