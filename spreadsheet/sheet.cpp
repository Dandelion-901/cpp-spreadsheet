#include "sheet.h"

#include "cell.h"
#include "common.h"

#include <algorithm>
#include <functional>
#include <iomanip>
#include <iostream>
#include <optional>
#include <sstream>

using namespace std::literals;

namespace {

static void CheckIfValid(Position pos) {
    if (!pos.IsValid()) {
        throw InvalidPositionException("Wrong position");
    }
}

static Sheet::Align GetCellAlign(int col, Cell& cell) {
    Sheet::Align align{};
    std::visit(
        [&align](auto&& args) {
            std::ostringstream txt_stream;
            txt_stream << args;
            align.val = txt_stream.str().size();
        }, cell.GetValue()
            );
    align.txt = cell.GetText().size();
    return align;
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
    align_.at(pos.col).Max(GetCellAlign(pos.col, *cell));
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

// Вырвнивание для номера строки
static const int ROW_NUM_ALIGN = 3;
// Базовое выравнивание для пустой ячейки
static const int BASIC_ALLIGN = 3;
// Разделитель ячеек
static const char DELIM = '|';

void Sheet::PrintValues(std::ostream& output) const {
    using namespace std;

    output << setw(ROW_NUM_ALIGN) << "c++";

    const Size scope = GetPrintableSize();
    for (int i{}; i < scope.cols; ++i) {
        const auto a = max(BASIC_ALLIGN, align_.at(i).val);
        output << DELIM
               << setw(a) << pos_convert::IndexToColumn(i);
    }
    output << endl;

    for (size_t row{}; row < scope.rows; ++row) {
        output << setw(ROW_NUM_ALIGN) << row + 1;
        for (size_t col{}; col < scope.cols; ++col) {
            output << DELIM;

            auto& cell = sheet_.at(row).at(col);
            const auto a = max(BASIC_ALLIGN, align_.at(col).val);
            if (cell) {
                visit(
                    [&output, &a](auto&& arg) {
                        output << setw(a) << arg;
                    }, cell->GetValue());
            }
            else {
                output << setw(a) << ' ';
            }
        }
        output << std::endl;
    }
}
void Sheet::PrintTexts(std::ostream& output) const {
    using namespace std;

    output << setw(ROW_NUM_ALIGN) << "c++";

    const Size scope = GetPrintableSize();
    for (int i{}; i < scope.cols; ++i) {
        const auto a = max(BASIC_ALLIGN, align_.at(i).txt);
        output << DELIM
               << setw(a) << pos_convert::IndexToColumn(i);
    }
    output << endl;

    for (size_t row{}; row < scope.rows; ++row) {
        output << right << setw(ROW_NUM_ALIGN) << row;
        for (size_t col{}; col < scope.cols; ++col) {
            output << DELIM;

            auto& cell = sheet_.at(row).at(col);
            const auto a = max(BASIC_ALLIGN, align_.at(col).txt);
            if (cell) {
                output << setw(a) << cell->GetText();
            }
            else {
                output << setw(a) << ' ';
            }
        }
        output << std::endl;
    }
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

void Sheet::FindAndSetMaxAlign(int col) {
    Align new_align{};
    const Size scope = GetPrintableSize();
    for (int i = 0; i < scope.rows; ++i) {
        const auto cell{ sheet_.at(i).at(col).get() };
        if (cell) {
            const Align cell_align{ GetCellAlign(col, *cell)};
            new_align.Max(cell_align);
        }
    }
    align_.at(col) = new_align;
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

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}

void Sheet::Align::Max(Align other) {
    val = std::max(val, other.val);
    txt = std::max(txt, other.txt);
}
