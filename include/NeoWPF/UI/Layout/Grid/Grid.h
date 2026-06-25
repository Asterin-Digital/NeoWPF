#pragma once

#include <NeoWPF/UI/Layout/Panel.h>
#include <vector>
#include <unordered_map>

namespace NeoWPF::UI {

class Grid : public Panel {
public:
    struct Definition {
        int size = 0; // simple fixed-size / auto placeholder
    };

    Grid() = default;

    void AddRow(int size = 0) { m_rowDefs.push_back({size}); }
    void AddColumn(int size = 0) { m_colDefs.push_back({size}); }

    static void SetRow(Visual* v, int r);
    static void SetColumn(Visual* v, int c);
    static int GetRow(Visual* v);
    static int GetColumn(Visual* v);

    void Measure(int availableWidth, int availableHeight) override;
    void Arrange(int finalWidth, int finalHeight) override;

private:
    std::vector<Definition> m_rowDefs;
    std::vector<Definition> m_colDefs;

    static std::unordered_map<Visual*, int> s_rowMap;
    static std::unordered_map<Visual*, int> s_colMap;
};

}
