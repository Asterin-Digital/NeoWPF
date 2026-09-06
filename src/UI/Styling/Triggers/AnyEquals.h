#pragma once

#include <any>
#include <string>

namespace NeoWPF::UI::Styling::Triggers::Detail {

inline bool AnyEquals(const std::any& left, const std::any& right) {
    if (left.type() != right.type()) {
        return false;
    }

    if (left.type() == typeid(bool)) {
        return std::any_cast<bool>(left) == std::any_cast<bool>(right);
    }
    if (left.type() == typeid(int)) {
        return std::any_cast<int>(left) == std::any_cast<int>(right);
    }
    if (left.type() == typeid(float)) {
        return std::any_cast<float>(left) == std::any_cast<float>(right);
    }
    if (left.type() == typeid(double)) {
        return std::any_cast<double>(left) == std::any_cast<double>(right);
    }
    if (left.type() == typeid(std::string)) {
        return std::any_cast<std::string>(left) == std::any_cast<std::string>(right);
    }

    return false;
}

}