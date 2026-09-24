/*
Apache 2.0 License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <string>
#include <stdexcept>

namespace NeoPM::Core::Resources {
    enum class ResourceType {
        Image,
        Font,
        Xaml,
        Raw
    };

    struct ResourceEntry {
        std::string key;           // e.g., "Icons/Play"
        ResourceType type;
        std::string path;          // relative to Assets/
        bool embed = true;         // true: embed in binary, false: sideload at runtime

        // Convert ResourceType enum to string for code generation
        static const char* TypeToString(ResourceType type) {
            switch (type) {
                case ResourceType::Image: return "Image";
                case ResourceType::Font:  return "Font";
                case ResourceType::Xaml:  return "Xaml";
                case ResourceType::Raw:   return "Raw";
            }
            return "Unknown";
        }

        // Convert string to ResourceType enum
        static ResourceType StringToType(const std::string& typeStr) {
            if (typeStr == "Image") return ResourceType::Image;
            if (typeStr == "Font")  return ResourceType::Font;
            if (typeStr == "Xaml")  return ResourceType::Xaml;
            if (typeStr == "Raw")   return ResourceType::Raw;
            throw std::invalid_argument("Unknown resource type: " + typeStr);
        }

        // Validate entry
        bool IsValid() const {
            return !key.empty() && !path.empty();
        }

        // Generate a valid C++ identifier from the key (e.g., "Icons/Play" -> "Icons_Play")
        std::string GetIdentifier() const {
            std::string id = key;
            for (auto& ch : id) {
                if (ch == '/' || ch == '-' || ch == '.') {
                    ch = '_';
                }
            }
            return id;
        }
    };
}
