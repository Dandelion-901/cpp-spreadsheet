#include "sheet.h"

#include "cell.h"
#include "common.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>

using namespace std::literals;

Sheet::~Sheet() = default;

void Sheet::SetCell(Position pos, std::string text) {
    if (!pos.IsValid())
        throw InvalidPositionException("Wrong position");

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
}

const CellInterface* Sheet::GetCell(Position pos) const {
    if (!pos.IsValid())
        throw InvalidPositionException("Wrong position");

    if (!IsInScope(pos))
        return nullptr;

    return sheet_.at(pos.row).at(pos.col).get();
}
CellInterface* Sheet::GetCell(Position pos) {
    if (!pos.IsValid())
        throw InvalidPositionException("Wrong position");

    if (!IsInScope(pos))
        return nullptr;

    return sheet_.at(pos.row).at(pos.col).get();
}

void Sheet::ClearCell(Position pos) {
    if (!pos.IsValid())
        throw InvalidPositionException("Wrong position");

    if (!IsInScope(pos))
        return;

    auto& cell(sheet_.at(pos.row).at(pos.col));
    cell->EraseDependencies();
    cell.reset();
    if (IsEdgePos(pos))
        RecomputeScope();
}

Size Sheet::GetPrintableSize() const {
    return scope_;
}

void Sheet::PrintValues(std::ostream& output) const {
    for (const auto& row : sheet_) {
        for (const auto& cell : row) {
            if (cell) {
                std::visit([&output](auto&& arg) { output << arg; }, cell->GetValue());
            }
            if (&row.back() != &cell)
                output << '\t';
        }
        output << std::endl;
    }
}
void Sheet::PrintTexts(std::ostream& output) const {
    for (const auto& row : sheet_) {
        for (const auto& cell : row) {
            if (cell) {
                output << cell->GetText();
            }
            if (&row.back() != &cell)
                output << '\t';
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
    sheet_.resize(val.rows);
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
                ns.rows = ns.rows < r ? r : ns.rows;
                ns.cols = ns.cols < c ? c : ns.cols;
                break;
            }
            else if (c < ns.cols)
                break;
        }
    }

    if (scope_ == ns)
        return;
    else
        ResizeScope(ns);
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}