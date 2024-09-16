#include "cell.h"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <string>
#include <optional>

namespace {

static bool IsNumber(const std::string& text) {
    if (text.empty()) {
        return false;
    }
    const auto not_digit_it = std::find_if_not(
        text.begin(), text.end(),
        [](const char ch) {
            return std::isdigit(ch);
        });

    return text.end() == not_digit_it;
}

}   // namespace

// public

Cell::Cell() : impl_(std::make_unique<EmptyImpl>()) {}

Cell::~Cell() = default;

void Cell::Set(std::string text, const SheetInterface* sheet) {
    auto new_cell = std::make_unique<Cell>();
    new_cell->sheet_ = sheet;

    if (!text.empty() && FORMULA_SIGN == text.front()) {
        new_cell->impl_ = std::make_unique<FormulaImpl>(text.substr(1));
    }
    else {
        new_cell->impl_ = std::make_unique<TextImpl>(text);
        ReleaseOldCell(*new_cell);
        return;
    }

    const auto ref = new_cell->impl_->GetReferences();
    for (Position pos : ref) {
        if (nullptr == sheet->GetCell(pos)) {
            const_cast<SheetInterface*>(sheet)->SetCell(pos, "");
        }
        new_cell->dependencies_.insert(
            reinterpret_cast<const Cell*>(sheet->GetCell(pos))
        );
    }

    //std::unordered_set<const Cell*> road;
    std::unordered_set<const Cell*> passed;
    passed.insert(this);
    new_cell->WalkByDependencies(passed);
    ReleaseOldCell(*new_cell);
}

void Cell::Clear() {
    Set("", sheet_);
}

Cell::Value Cell::GetValue() const {
    return impl_->GetValue(*sheet_);
}
std::string Cell::GetText() const {
    return impl_->GetText();
}

std::vector<Position> Cell::GetReferencedCells() const {
    return impl_->GetReferences();
}

bool Cell::IsReferenced() const {
    return !dependants_.empty();
}

void Cell::EraseDependencies() {
    for (auto& c : dependencies_) {
        c->dependants_.erase(this);
    }
}

// private

void Cell::ReleaseOldCell(Cell& new_cell) {
    EraseDependencies();
    for (auto& c : new_cell.dependencies_) {
        c->dependants_.insert(this);
    }

    std::swap(sheet_, new_cell.sheet_);
    std::swap(impl_, new_cell.impl_);
    std::swap(dependencies_, new_cell.dependencies_);
}

void Cell::WalkByDependencies(std::unordered_set<const Cell*>& passed) const {
    if (passed.count(this)) {
        throw CircularDependencyException("Circular dependency found");
    }
    if (dependencies_.empty()) {
        return;
    }

    passed.insert(this);
    for (auto& c : dependencies_) {
        c->WalkByDependencies(passed);
    }
}

void Cell::InvalidateValue() const {
    if (typeid(FormulaImpl) == typeid(impl_)) {
        impl_->Invalidate();
        for (auto& c : dependants_) {
            c->InvalidateValue();
        }
    }
}

// EmptyImpl
std::string Cell::EmptyImpl::GetText() const {
    return "";
}
CellInterface::Value Cell::EmptyImpl::GetValue(const SheetInterface&) const {
    return Value();
}
std::vector<Position> Cell::EmptyImpl::GetReferences() const {
    return {};
}
void Cell::EmptyImpl::Invalidate() const {}

// TextImpl
Cell::TextImpl::TextImpl(std::string input)
    : text_(input)
{}
std::string Cell::TextImpl::GetText() const {
    return text_;
}
CellInterface::Value Cell::TextImpl::GetValue(const SheetInterface&) const {
    if (!text_.empty() && ESCAPE_SIGN == text_.front()) {
        return text_.substr(1);
    }
    else if (IsNumber(text_)) {
        return std::stod(text_);
    }
    else {
        return text_;
    }
}
std::vector<Position> Cell::TextImpl::GetReferences() const {
    return {};
}
void Cell::TextImpl::Invalidate() const {}

// FormulaImpl
Cell::FormulaImpl::FormulaImpl(std::string input)
    : expr_(ParseFormula(input))
{}
std::string Cell::FormulaImpl::GetText() const {
    return FORMULA_SIGN + expr_->GetExpression();
}
CellInterface::Value Cell::FormulaImpl::GetValue(const SheetInterface& sheet) const {
    if (!cache_.has_value()) {
        const auto val = expr_->Evaluate(sheet);
        if (std::holds_alternative<double>(val)) {
            cache_.emplace(std::get<double>(val));
        }
        else {
            cache_.emplace(std::get<FormulaError>(val));
        }
    }
    return cache_.value();
}
std::vector<Position> Cell::FormulaImpl::GetReferences() const {
    return expr_->GetReferencedCells();
}
void Cell::FormulaImpl::Invalidate() const {
    cache_.reset();
}
