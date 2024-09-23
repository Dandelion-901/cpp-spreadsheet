#include "sheet.h"

#include "cell.h"
#include "common.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>

using namespace std::literals;

namespace {

static void CheckIfValid(Position pos) {
    if (!pos.IsValid()) {
        throw InvalidPositionException("Wrong position");
    }
}

}   // namespace

Sheet::~Sheet() = default;

void Sheet::SetCell(Position pos, std::string text) {
    CheckIfValid(pos);

    if (!IsInScope(pos)) {
        Size new_scope{
            pos.row < scope_.rows ? scope_.rows : pos.row + 1,
            pos.col < scope_.cols ? scope_.cols : pos.col + 1
        };
        ResizeScope(new_scope);
    }

    auto& cell = sheet_.at(pos.row).at(pos.col);
    if (!cell) {
        cell = std::make_unique<Cell>();
    }
    cell->Set(text, this);
    align_.at(pos.col).Max(sheet_draw::GetCellAlign(pos.col, GetConcreteCell(pos)));
}

const CellInterface* Sheet::GetCell(Position pos) const {
    CheckIfValid(pos);

    if (!IsInScope(pos)) {
        return nullptr;
    }

    return sheet_.at(pos.row).at(pos.col).get();
}
CellInterface* Sheet::GetCell(Position pos) {
    CheckIfValid(pos);

    if (!IsInScope(pos)) {
        return nullptr;
    }

    return sheet_.at(pos.row).at(pos.col).get();
}

void Sheet::ClearCell(Position pos) {
    CheckIfValid(pos);

    if (!IsInScope(pos)) {
        return;
    }

    auto& cell(sheet_.at(pos.row).at(pos.col));
    if (cell) {
        cell->Clear();

        cell.reset();
        if (IsEdgePos(pos)) {
            RecomputeScope();
        }
        FindAndSetMaxAlign(pos.col);
    }
}

Size Sheet::GetPrintableSize() const {
    return scope_;
}

void Sheet::PrintValues(std::ostream& output) const {
    PrintCells(output, false);
}
void Sheet::PrintTexts(std::ostream& output) const {
    PrintCells(output, true);
}

void Sheet::DrawSheet(std::ostream& output, bool is_text) const {
    using namespace sheet_draw;

    SheetDrawer drawer(output, align_);

    //drawer.DrawEdgeLine(is_text);
    drawer.DrawHeader(is_text);
    for (int i{}; i < scope_.rows; ++i) {
        drawer.DrawDelimLine(is_text);
        drawer.DrawRow(i + 1, sheet_.at(i), is_text);
    }
    //drawer.DrawEdgeLine(is_text);
}

const Cell* Sheet::GetConcreteCell(Position pos) const {
    return sheet_.at(pos.row).at(pos.col).get();
}

Cell* Sheet::GetConcreteCell(Position pos) {
    return sheet_.at(pos.row).at(pos.col).get();
}

bool Sheet::IsInScope(Position pos) const {
    return pos.row < scope_.rows && pos.col < scope_.cols;
}

bool Sheet::IsEdgePos(Position pos) const {
    return pos.row + 1 == scope_.rows || pos.col + 1 == scope_.cols;
}

void Sheet::ResizeScope(Size val) {
    if (scope_ == val) {
        return;
    }
    sheet_.resize(val.rows);
    align_.resize(val.cols);
    for (auto& row : sheet_) {
        row.resize(val.cols);
    }
    scope_ = val;
}

void Sheet::RecomputeScope() {
    // new scope
    Size ns{};

    for (int r = scope_.rows; r != 0; --r) {
        for (int c = scope_.cols; c != 0; --c) {
            if (sheet_.at(r - 1).at(c - 1)) {
                ns.rows = std::max(ns.rows, r);
                ns.cols = std::max(ns.cols, c);
                break;
            }
            else if (c < ns.cols) {
                break;
            }
        }
    }

    ResizeScope(ns);
    return;
}

void Sheet::FindAndSetMaxAlign(int col) {
    if (align_.empty()) {
        return;
    }

    sheet_draw::Align new_align{};
    for (int i = 0; i < scope_.rows; ++i) {
        const auto cell{ sheet_.at(i).at(col).get() };
        new_align.Max(sheet_draw::GetCellAlign(col, cell));
    }
    align_.at(col) = new_align;
}

void Sheet::PrintCells(std::ostream& output, bool is_text) const {
    for (const auto& row : sheet_) {
        for (const auto& cell : row) {
            if (cell) {
                if (is_text) {
                    output << cell->GetText();
                }
                else {
                    std::visit(
                        [&output](auto&& arg) {
                            output << arg;
                        }, cell->GetValue());
                }
            }
            if (&row.back() != &cell) {
                output << '\t';
            }
        }
        output << std::endl;
    }
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}
