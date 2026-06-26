#pragma once

#include <NeoWPF/UI/Layout/Panel/Panel.h>
#include <vector>
#include <unordered_map>

namespace NeoWPF::UI {

enum class SizeMode {
    Auto,
    Fixed,
    Star
};

class Grid : public Panel {
public:
    struct Definition {
        SizeMode mode = SizeMode::Auto;
        int value = 0;
    };

    Grid() = default;

    void AddRow(SizeMode mode = SizeMode::Auto, int value = 0) { m_rowDefs.push_back({mode, value}); }
    void AddColumn(SizeMode mode = SizeMode::Auto, int value = 0) { m_colDefs.push_back({mode, value}); }

    static void SetRow(Visual* v, int r);
    static void SetColumn(Visual* v, int c);
    static int GetRow(Visual* v);
    static int GetColumn(Visual* v);

    static void SetRowSpan(Visual* v, int span);
    static void SetColumnSpan(Visual* v, int span);
    static int GetRowSpan(Visual* v);
    static int GetColumnSpan(Visual* v);

    void Measure(int availableWidth, int availableHeight) override;
    void Arrange(int finalWidth, int finalHeight) override;

private:
    std::vector<Definition> m_rowDefs;
    std::vector<Definition> m_colDefs;

    static std::unordered_map<Visual*, int> s_rowMap;
    static std::unordered_map<Visual*, int> s_colMap;
    static std::unordered_map<Visual*, int> s_rowSpanMap;
    static std::unordered_map<Visual*, int> s_colSpanMap;
};

}
