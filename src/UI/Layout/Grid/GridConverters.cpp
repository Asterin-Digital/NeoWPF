#include <NeoWPF/UI/Layout/Grid/Grid.h>
#include <NeoWPF/UI/Layout/Grid/RowDefinition.h>
#include <NeoWPF/UI/Layout/Grid/ColumnDefinition.h>
#include <NeoWPF/UI/Properties/TypeConverter.h>
#include <NeoWPF/Platform/PlatformUtils.h>

#include <algorithm>
#include <cctype>
#include <stdexcept>
#include <string>
#include <string_view>

namespace {

static std::string TrimAndLower(std::string_view value)
{
    const auto trimmed = NeoWPF::Platform::Trim(std::string(value));
    return NeoWPF::Platform::ToLower(trimmed);
}

bool ParseGridLength(std::string value, NeoWPF::UI::Grid::GridLength& outLength)
{
    if (value.empty()) {
        return false;
    }

    value = NeoWPF::Platform::Trim(value);
    if (value.empty()) {
        return false;
    }

    const auto lower = TrimAndLower(value);
    if (lower == "auto") {
        outLength = NeoWPF::UI::Grid::GridLength::Auto();
        return true;
    }

    if (lower == "*") {
        outLength = NeoWPF::UI::Grid::GridLength::Star();
        return true;
    }

    try {
        const int pixels = std::stoi(lower);
        outLength = NeoWPF::UI::Grid::GridLength::Fixed(pixels);
        return true;
    } catch (...) {
        return false;
    }
}

class GridLengthConverter final : public NeoWPF::UI::Properties::TypeConverter {
public:
    bool ConvertFromString(const std::string& rawValue, std::any& convertedValue) const override
    {
        NeoWPF::UI::Grid::GridLength length{};
        if (!ParseGridLength(rawValue, length)) {
            return false;
        }

        convertedValue = length;
        return true;
    }
};

class GridDefinitionModeConverter final : public NeoWPF::UI::Properties::TypeConverter {
public:
    bool ConvertFromString(const std::string& rawValue, std::any& convertedValue) const override
    {
        NeoWPF::UI::Grid::GridLength length{};
        if (!ParseGridLength(rawValue, length)) {
            return false;
        }

        convertedValue = static_cast<int>(length.Mode);
        return true;
    }
};

}

namespace NeoWPF::UI::Properties {

void RegisterGridConverters()
{
    static const auto gridLengthConverter = std::make_shared<GridLengthConverter>();
    static const auto gridDefinitionModeConverter = std::make_shared<GridDefinitionModeConverter>();


    (void)gridLengthConverter;
    (void)gridDefinitionModeConverter;
}

}
