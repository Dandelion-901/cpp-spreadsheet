#pragma once

#include "cell.h"
#include "common.h"

#include <vector>

#include <functional>

class Sheet : public SheetInterface {
public:
    ~Sheet();

    void SetCell(Position pos, std::string text) override;

    const CellInterface* GetCell(Position pos) const override;
    CellInterface* GetCell(Position pos) override;

    void ClearCell(Position pos) override;

    Size GetPrintableSize() const override;

    void PrintValues(std::ostream& output) const override;
    void PrintTexts(std::ostream& output) const override;

    const Cell* GetConcreteCell(Position pos) const;
    Cell* GetConcreteCell(Position pos);

    struct Align {
        int val;
        int txt;

        // Change Align if input bigger
        void Max(Align other);
    };

private:
    using Row = std::vector<std::unique_ptr<Cell>>;
    using Table = std::vector<Row>;

    bool IsInScope(Position pos) const;
    bool IsEdgePos(Position pos) const;
    void ResizeScope(Size val);
    void FindAndSetMaxAlign(int col);
    void RecomputeScope();

    Size scope_;
    Table sheet_;
    std::vector<Align> align_;
};