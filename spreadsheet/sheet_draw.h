#pragma once

#include "cell.h"
#include "common.h"

#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>

namespace sheet_draw {

using namespace std;

// Вырвнивание для номера строки
static constexpr int ROW_ID_ALIGN = 3;
// Базовое выравнивание для пустой ячейки
static constexpr int BASIC_ALLIGN = 3;
// Разделитель ячеек
static const char DELIM('|');

struct Align {
    int val{ BASIC_ALLIGN };
    int txt{ BASIC_ALLIGN };

    // Change Align if input bigger
    void Max(Align other) {
        val = max(val, other.val);
        txt = max(txt, other.txt);
    }
};

static Align GetCellAlign(int col, const Cell* cell) {
    Align align{};

    const int col_id_size = pos_convert::IndexToColumn(col).size();
    align.Max({ col_id_size, col_id_size });

    if (cell) {
        visit(
            [&align](auto&& args) {
                ostringstream txt_stream;
                txt_stream << args;
                align.val = max(align.val, static_cast<int>(txt_stream.str().size()));
            }, cell->GetValue()
                );
        align.txt = max(align.txt, static_cast<int>(cell->GetText().size()));
    }
    return align;
}

class SheetDrawer {
private:
    ostream& out_;
    const vector<Align>& align_;
public:
    SheetDrawer() = delete;
    SheetDrawer(SheetDrawer&) = delete;
    explicit SheetDrawer(ostream& output, const vector<Align>& align)
        : out_(output)
        , align_(align)
    {}

    SheetDrawer& operator=(const SheetDrawer&) = delete;
    SheetDrawer& operator=(SheetDrawer&&) = default;

    ~SheetDrawer() = default;

    void DrawEdgeLine(bool is_text) const {
        out_ << ' ' << string(ROW_ID_ALIGN, '-');
        for (auto& a : align_) {
            if (is_text) {
                out_ << string(a.txt + 1, '-');
            }
            else {
                out_ << string(a.val + 1, '-');
            }
        }
        out_ << ' '
             << endl;
    }

    void DrawDelimLine(bool is_text) const {
        out_ /*  << '|' */ << setfill('-')
             << setw(ROW_ID_ALIGN) << "";
        for (auto& a : align_) {
            out_ << '+';
            if (is_text) {
                out_ << setw(a.txt);
            }
            else {
                out_ << setw(a.val);
            }
            out_ << "";
        }
        out_ /* << '|' */ << setfill(' ')
             << endl;
    }

    void DrawHeader(bool is_text) const {
        out_ /* << '|' */ << setw(ROW_ID_ALIGN) << "c++";
        for (int i{}; i < align_.size(); ++i) {
            out_ << DELIM;
            int a{};
            if (is_text) {
                a = align_.at(i).txt;
            }
            else {
                a = align_.at(i).val;
            }
            const auto index = pos_convert::IndexToColumn(i);
            a = index.size() < a ? a - index.size() : 0;
            out_ << string(a / 2, ' ')
                 << index
                 << string(a - (a / 2), ' ');
        }
        out_ /* << '|' */ << endl;
    }

    void DrawCell(int col, const Cell* cell, bool is_text) const {
        out_ << DELIM;
        if (cell) {
            if (is_text) {
                out_ << setw(align_.at(col).txt)
                     << cell->GetText();
            }
            else {
                out_ << setw(align_.at(col).val);
                visit(
                    [&out = this->out_](auto&& arg) {
                        out << arg;
                    }, cell->GetValue());
            }
        }
        else {
            if (is_text) {
                out_ << setw(align_.at(col).txt) << "";
            }
            else {
                out_ << setw(align_.at(col).val) << "";
            }
        }
    }

    using Row = std::vector<std::unique_ptr<Cell>>;
    void DrawRow(int row_id, const Row& row, bool is_text) const {
        out_ /* << '|' */ << setw(ROW_ID_ALIGN) << row_id;
        for (int i{}; i < row.size(); ++i) {
            DrawCell(i, row.at(i).get(), is_text);
        }
        out_ /* << '|' */ << endl;
    }
};

}
