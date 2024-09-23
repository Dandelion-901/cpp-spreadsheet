#pragma once

#include "cell.h"
#include "common.h"
#include "sheet_draw.h"

#include <vector>

#include <functional>

class Sheet : public SheetInterface {
public:
    using Row = std::vector<std::unique_ptr<Cell>>;
    using Table = std::vector<Row>;

    ~Sheet();

    void SetCell(Position pos, std::string text) override;

    const CellInterface* GetCell(Position pos) const override;
    CellInterface* GetCell(Position pos) override;

    void ClearCell(Position pos) override;

    Size GetPrintableSize() const override;

    void PrintValues(std::ostream& output) const override;
    void PrintTexts(std::ostream& output) const override;

    void DrawSheet(std::ostream& output, bool is_text) const;

    const Cell* GetConcreteCell(Position pos) const;
    Cell* GetConcreteCell(Position pos);

private:
    bool IsInScope(Position pos) const;
    bool IsEdgePos(Position pos) const;

    void ResizeScope(Size val);
    void RecomputeScope();

    void FindAndSetMaxAlign(int col);

    void PrintCells(std::ostream& output, bool is_text) const;

    Size scope_;
    Table sheet_;
    std::vector<sheet_draw::Align> align_;
};