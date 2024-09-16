#pragma once

#include "common.h"
#include "formula.h"

#include <optional>
#include <unordered_set>

class Cell : public CellInterface {
public:
    Cell();
    ~Cell();

    void Set(std::string text, const SheetInterface* sheet);
    void Clear();

    Value GetValue() const override;
    std::string GetText() const override;

    std::vector<Position> GetReferencedCells() const override;
    bool IsReferenced() const;
    void EraseDependencies();

private:
    void ReleaseOldCell(Cell& old_cell);
    void WalkByDependencies(std::unordered_set<const Cell*>& passed) const;
    void InvalidateValue() const;

    class Impl {
    public:
        virtual Value GetValue(const SheetInterface&) const = 0;
        virtual std::string GetText() const = 0;
        virtual std::vector<Position> GetReferences() const = 0;
        virtual void Invalidate() const = 0;
    };

    class EmptyImpl : public Impl {
    public:
        std::string GetText() const override;
        Value GetValue(const SheetInterface&) const override;
        std::vector<Position> GetReferences() const override;
        void Invalidate() const override;
    };

    class TextImpl : public Impl {
    private:
        std::string text_;
    public:
        explicit TextImpl(std::string input);
        std::string GetText() const override;
        Value GetValue(const SheetInterface&) const override;
        std::vector<Position> GetReferences() const override;
        void Invalidate() const override;
    };

    class FormulaImpl : public Impl {
    private:
        std::unique_ptr<FormulaInterface> expr_;
        mutable std::optional<Value> cache_;
    public:
        explicit FormulaImpl(std::string input);
        std::string GetText() const override;
        Value GetValue(const SheetInterface& sheet) const override;
        std::vector<Position> GetReferences() const override;
        void Invalidate() const override;
    };

    const SheetInterface* sheet_;
    std::unique_ptr<Impl> impl_;

    mutable std::unordered_set<const Cell*> dependencies_;
    mutable std::unordered_set<const Cell*> dependants_;
};