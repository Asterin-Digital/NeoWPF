#include <NeoWPF/Core/Application/EntryRunner.h>

#include <NeoWPF/Feedback/Result.h>
#include <NeoWPF/Platform/PlatformUtils.h>

#include <filesystem>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace NeoWPF::Platform::Linux {
NeoWPF::Result<std::filesystem::path> ResolveExecutableDirectory();
}

namespace {

NeoWPF::Result<std::vector<std::wstring>> GetArguments(int argc, char** argv)
{
    if (argc < 0 || (argc > 0 && !argv)) {
        return NeoWPF::Fail<std::string>{"Linux supplied an invalid argument array."};
    }

    std::vector<std::wstring> arguments;
    arguments.reserve(static_cast<std::size_t>(argc));
    for (int index = 0; index < argc; ++index) {
        if (!argv[index]) {
            return NeoWPF::Fail<std::string>{
                "Linux supplied a null argument at index " + std::to_string(index) + "."};
        }

        auto converted = NeoWPF::Platform::Utf8ToWide(std::string(argv[index]));
        if (converted.Fail()) {
            return NeoWPF::Fail<std::string>{
                "Failed to convert Linux argument " + std::to_string(index) + ": " +
                converted.Error()};
        }
        arguments.push_back(std::move(converted).Value());
    }
    return NeoWPF::Success<std::vector<std::wstring>>{std::move(arguments)};
}

}

int main(int argc, char** argv)
{
    return NeoWPF::Core::EntryRunner::Run(
        GetArguments(argc, argv),
        NeoWPF::Success<std::optional<std::string>>{std::nullopt},
        NeoWPF::Platform::Linux::ResolveExecutableDirectory());
}
