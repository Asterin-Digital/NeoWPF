#include <NeoWPF/UI/Layout/Grid/Grid.h>
#include <NeoWPF/UI/Layout/Grid/GridDefinitionConverters.h>
#include <NeoWPF/UI/Properties/TypeConverter.h>
#include <NeoWPF/Platform/PlatformUtils.h>

#include <string>

namespace {

bool ParseGridLengthValue(const std::string& rawValue, NeoWPF::UI::Grid::GridLength& length)
{
    std::string value = NeoWPF::Platform::Trim(rawValue);
    if (value.empty()) {
        return false;
    }

    const std::string lowered = NeoWPF::Platform::ToLower(value);
    if (lowered == "auto") {
        length = NeoWPF::UI::Grid::GridLength::Auto();
        return true;
    }
    if (lowered == "*") {
        length = NeoWPF::UI::Grid::GridLength::Star();
        return true;
    }
    if (lowered.size() > 1 && lowered.back() == '*') {
        const std::string weight = lowered.substr(0, lowered.size() - 1);
        try {
            const int starWeight = std::stoi(weight);
            length = NeoWPF::UI::Grid::GridLength::Star(starWeight);
            return true;
        } catch (...) {
            return false;
        }
    }

    try {
        const int pixels = std::stoi(lowered);
        length = NeoWPF::UI::Grid::GridLength::Fixed(pixels);
        return true;
    } catch (...) {
        return false;
    }
}

class GridLengthValueConverter final : public NeoWPF::UI::Properties::TypeConverter {
public:
    bool ConvertFromString(const std::string& rawValue, std::any& convertedValue) const override
    {
        NeoWPF::UI::Grid::GridLength length{};
        if (!ParseGridLengthValue(rawValue, length)) {
            return false;
        }

        convertedValue = length;
        return true;
    }
};

}

namespace NeoWPF::UI::Properties {

std::shared_ptr<TypeConverter> GetGridLengthConverter()
{
    static const auto converter = std::make_shared<GridLengthValueConverter>();
    return converter;
}

class GridDefinitionModeConverter final : public TypeConverter {
public:
    bool ConvertFromString(const std::string& rawValue, std::any& convertedValue) const override
    {
        NeoWPF::UI::Grid::GridLength length{};
        if (!ParseGridLengthValue(rawValue, length)) {
            return false;
        }

        convertedValue = static_cast<int>(length.Mode);
        return true;
    }
};

std::shared_ptr<TypeConverter> GetGridDefinitionModeConverter()
{
    static const auto converter = std::make_shared<GridDefinitionModeConverter>();
    return converter;
}

}
