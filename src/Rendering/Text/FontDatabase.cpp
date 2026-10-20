#include <NeoWPF/Rendering/Text/FontDatabase.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <algorithm>
#include <atomic>
#include <cctype>
#include <fstream>
#include <mutex>
#include <string>
#include <utility>

namespace NeoWPF::Rendering::Text {

namespace {

std::string NormalizeFamily(std::string family)
{
    std::string normalized;
    normalized.reserve(family.size());
    bool pendingSpace = false;
    for (const auto character : family) {
        const auto value = static_cast<unsigned char>(character);
        if (std::isspace(value)) {
            pendingSpace = !normalized.empty();
            continue;
        }
        if (pendingSpace) {
            normalized.push_back(' ');
            pendingSpace = false;
        }
        normalized.push_back(static_cast<char>(std::tolower(value)));
    }
    return normalized;
}

}

struct FontDatabase::Impl {
    struct Entry {
        std::shared_ptr<const FontRecord> record;
        FT_Face face = nullptr;
        std::shared_ptr<const std::vector<std::byte>> data;
    };

    FT_Library library = nullptr;
    std::vector<Entry> entries;
    mutable std::mutex mutex;
    std::atomic<std::uint64_t> generation = 0;
};

FontDatabase::FontDatabase()
    : m_impl(std::make_unique<Impl>())
{
    if (FT_Init_FreeType(&m_impl->library) != 0) {
        m_impl->library = nullptr;
    }
}

FontDatabase::~FontDatabase()
{
    if (!m_impl || !m_impl->library) {
        return;
    }

    for (auto& entry : m_impl->entries) {
        if (entry.face) {
            FT_Done_Face(entry.face);
        }
    }
    FT_Done_FreeType(m_impl->library);
}

bool FontDatabase::RegisterFile(const std::filesystem::path& path)
{
    if (path.empty() || !m_impl->library) {
        return false;
    }

    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file) {
        return false;
    }

    const auto size = file.tellg();
    if (size <= 0) {
        return false;
    }
    file.seekg(0, std::ios::beg);

    auto data = std::make_shared<std::vector<std::byte>>(static_cast<std::size_t>(size));
    if (!file.read(reinterpret_cast<char*>(data->data()), size)) {
        return false;
    }

    return RegisterMemory(std::move(data), {});
}

bool FontDatabase::RegisterMemory(std::shared_ptr<const std::vector<std::byte>> data,
                                  std::string familyUtf8)
{
    if (!m_impl->library || !data || data->empty()) {
        return false;
    }

    const auto* bytes = reinterpret_cast<const FT_Byte*>(data->data());
    FT_Face probe = nullptr;
    if (FT_New_Memory_Face(m_impl->library, bytes, static_cast<FT_Long>(data->size()), -1, &probe) != 0 ||
        !probe || probe->num_faces <= 0) {
        if (probe) {
            FT_Done_Face(probe);
        }
        return false;
    }
    const auto faceCount = probe->num_faces;
    FT_Done_Face(probe);

    std::lock_guard<std::mutex> lock(m_impl->mutex);
    bool registered = false;
    for (long faceIndex = 0; faceIndex < faceCount; ++faceIndex) {
        FT_Face face = nullptr;
        if (FT_New_Memory_Face(m_impl->library, bytes, static_cast<FT_Long>(data->size()), faceIndex, &face) != 0 ||
            !face) {
            continue;
        }

        std::string recordFamily = familyUtf8;
        if (recordFamily.empty() && face->family_name) {
            recordFamily = face->family_name;
        }
        if (recordFamily.empty()) {
            FT_Done_Face(face);
            continue;
        }

        const auto duplicate = std::find_if(m_impl->entries.begin(), m_impl->entries.end(), [&](const Impl::Entry& entry) {
            return NormalizeFamily(entry.record->familyUtf8) == NormalizeFamily(recordFamily) &&
                   entry.record->faceIndex == static_cast<unsigned int>(faceIndex) &&
                   entry.record->data == data;
        });
        if (duplicate != m_impl->entries.end()) {
            FT_Done_Face(face);
            continue;
        }

        auto record = std::make_shared<FontRecord>();
        record->familyUtf8 = std::move(recordFamily);
        record->data = data;
        record->faceIndex = static_cast<unsigned int>(faceIndex);
        record->style = (face->style_flags & FT_STYLE_FLAG_ITALIC)
            ? NeoWPF::UI::FontStyle::Italic
            : NeoWPF::UI::FontStyle::Normal;
        m_impl->entries.push_back({std::move(record), face, data});
        registered = true;
    }
    if (registered) {
        m_impl->generation.fetch_add(1, std::memory_order_release);
    }
    return registered;
}

std::shared_ptr<const FontRecord> FontDatabase::Resolve(FontRequest request) const
{
    std::lock_guard<std::mutex> lock(m_impl->mutex);
    const auto requestedFamily = NormalizeFamily(request.familyUtf8);
    const auto matches = [&](const Impl::Entry& entry) {
        return NormalizeFamily(entry.record->familyUtf8) == requestedFamily &&
               entry.record->style == request.style;
    };

    auto it = std::find_if(m_impl->entries.begin(), m_impl->entries.end(), matches);
    if (it == m_impl->entries.end()) {
        it = std::find_if(m_impl->entries.begin(), m_impl->entries.end(), [&](const Impl::Entry& entry) {
            return NormalizeFamily(entry.record->familyUtf8) == requestedFamily;
        });
    }
    if (it != m_impl->entries.end()) {
        auto best = it;
        const auto weightDistance = [&](const Impl::Entry& entry) {
            return std::abs(entry.record->weight - request.weight);
        };
        for (auto candidate = std::next(it); candidate != m_impl->entries.end(); ++candidate) {
            if (NormalizeFamily(candidate->record->familyUtf8) == requestedFamily &&
                candidate->record->style == request.style &&
                weightDistance(*candidate) < weightDistance(*best)) {
                best = candidate;
            }
        }
        it = best;
    }
    if (it == m_impl->entries.end() && !m_impl->entries.empty()) {
        it = m_impl->entries.begin();
    }
    return it == m_impl->entries.end() ? nullptr : it->record;
}

std::vector<std::shared_ptr<const FontRecord>> FontDatabase::Records() const
{
    std::lock_guard<std::mutex> lock(m_impl->mutex);
    std::vector<std::shared_ptr<const FontRecord>> records;
    records.reserve(m_impl->entries.size());
    for (const auto& entry : m_impl->entries) {
        records.push_back(entry.record);
    }
    return records;
}

std::uint64_t FontDatabase::Generation() const noexcept
{
    return m_impl->generation.load(std::memory_order_acquire);
}

FontDatabase& GetFontDatabase()
{
    static FontDatabase database;
    return database;
}

}
