#pragma once

#include <NeoWPF/UI/Layout/Panel/Panel.h>
#include <initializer_list>
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
    using Panel::AddChild;

    struct Definition {
        SizeMode mode = SizeMode::Auto;
        int value = 0;
    };

    Grid() = default;

    void AddRow(SizeMode mode = SizeMode::Auto, int value = 0) { m_rowDefs.push_back({mode, value}); }
    void AddRows(std::initializer_list<Definition> defs) { m_rowDefs.insert(m_rowDefs.end(), defs.begin(), defs.end()); }
    void AddColumn(SizeMode mode = SizeMode::Auto, int value = 0) { m_colDefs.push_back({mode, value}); }
    void AddColumns(std::initializer_list<Definition> defs) { m_colDefs.insert(m_colDefs.end(), defs.begin(), defs.end()); }

    void AddChild(Visual* child, int row, int column, int rowSpan = 1, int columnSpan = 1);

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
    std::vector<int> m_autoRowHeights;
    std::vector<int> m_autoColWidths;

    static std::unordered_map<Visual*, int> s_rowMap;
    static std::unordered_map<Visual*, int> s_colMap;
    static std::unordered_map<Visual*, int> s_rowSpanMap;
    static std::unordered_map<Visual*, int> s_colSpanMap;
};

}
