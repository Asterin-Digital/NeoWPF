#include <NeoWPF/Rendering/Text/FontDatabase.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <algorithm>
#include <fstream>
#include <mutex>
#include <utility>

namespace NeoWPF::Rendering::Text {

struct FontDatabase::Impl {
    struct Entry {
        std::shared_ptr<const FontRecord> record;
        FT_Face face = nullptr;
        std::shared_ptr<const std::vector<std::byte>> data;
    };

    FT_Library library = nullptr;
    std::vector<Entry> entries;
    mutable std::mutex mutex;
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

    FT_Face face = nullptr;
    const auto* bytes = reinterpret_cast<const FT_Byte*>(data->data());
    if (FT_New_Memory_Face(m_impl->library, bytes, static_cast<FT_Long>(data->size()), 0, &face) != 0) {
        return false;
    }

    if (familyUtf8.empty() && face->family_name) {
        familyUtf8 = face->family_name;
    }
    if (familyUtf8.empty()) {
        FT_Done_Face(face);
        return false;
    }

    auto record = std::make_shared<FontRecord>();
    record->familyUtf8 = std::move(familyUtf8);
    record->data = data;
    record->weight = 400;
    record->style = (face->style_flags & FT_STYLE_FLAG_ITALIC)
        ? NeoWPF::UI::FontStyle::Italic
        : NeoWPF::UI::FontStyle::Normal;

    std::lock_guard<std::mutex> lock(m_impl->mutex);
    m_impl->entries.push_back({std::move(record), face, std::move(data)});
    return true;
}

std::shared_ptr<const FontRecord> FontDatabase::Resolve(FontRequest request) const
{
    std::lock_guard<std::mutex> lock(m_impl->mutex);
    const auto matches = [&](const Impl::Entry& entry) {
        return entry.record->familyUtf8 == request.familyUtf8 &&
               entry.record->style == request.style;
    };

    auto it = std::find_if(m_impl->entries.begin(), m_impl->entries.end(), matches);
    if (it == m_impl->entries.end()) {
        it = std::find_if(m_impl->entries.begin(), m_impl->entries.end(), [&](const Impl::Entry& entry) {
            return entry.record->familyUtf8 == request.familyUtf8;
        });
    }
    if (it != m_impl->entries.end()) {
        auto best = it;
        const auto weightDistance = [&](const Impl::Entry& entry) {
            return std::abs(entry.record->weight - request.weight);
        };
        for (auto candidate = std::next(it); candidate != m_impl->entries.end(); ++candidate) {
            if (candidate->record->familyUtf8 == request.familyUtf8 &&
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

FontDatabase& GetFontDatabase()
{
    static FontDatabase database;
    return database;
}

}
