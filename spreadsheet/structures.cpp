#include "common.h"

#include <algorithm>
#include <cctype>
#include <sstream>

using namespace std::literals;

const int LETTERS = 26;
const int MAX_POSITION_LENGTH = 17;
const int MAX_POS_LETTER_COUNT = 3;

const Position Position::NONE = {-1, -1};

namespace {

    static int ColumnToIndex(const std::string_view col) {
        int index = 0;
        for (char c : col) {
            if (!std::isupper(c)) {
                return -1;
            }
            index = index * LETTERS + (c - 'A' + 1);
        }
        return --index;
    }
    static std::string IndexToColumn(int index) {
        std::string col;
        while (0 <= index) {
            col = static_cast<char>(index % LETTERS + 'A') + col;
            index = index / LETTERS - 1;
        }
        return col;
    }

}   // namespace

bool Position::operator==(Position rhs) const {
    return row == rhs.row && col == rhs.col;
}
bool Position::operator!=(Position rhs) const {
    return row != rhs.row && col != rhs.col;
}
bool Position::operator<(Position rhs) const {
    return col < rhs.col || (col == rhs.col && row < rhs.row);
}

bool Position::IsValid() const {
    return col >= 0 && col < MAX_COLS
        && row >= 0 && row < MAX_ROWS;
}

std::string Position::ToString() const {
    if (IsValid()) {
        return IndexToColumn(col) + std::to_string(row + 1);
    }
    else {
        return ""s;
    }
}

Position Position::FromString(std::string_view str) {
    Position pos = Position::NONE;
    if (str.empty() || MAX_POSITION_LENGTH < str.size()) {
        return pos;
    }

    const auto row_pos =
        std::find_if(str.begin(), str.end(),
            [](char c) {
                return std::isdigit(c);
            });
    if (str.end() == row_pos) {
        return pos;
    }
    const auto end_pos =
        std::find_if_not(row_pos, str.end(),
            [](char c) {
                return std::isdigit(c);
            });
    if (str.end() != end_pos) {
        return pos;
    }

    const size_t row_id = std::distance(str.begin(), row_pos);
    pos.col = ColumnToIndex(str.substr(0, row_id));
    pos.row = std::stoi(std::string(str.substr(row_id))) - 1;

    if (pos.IsValid()) {
        return pos;
    }
    else {
        return Position::NONE;
    }
}

bool Size::operator==(Size rhs) const {
    return rows == rhs.rows && cols == rhs.cols;
}

FormulaError::FormulaError(Category category)
    : category_(category)
{}

FormulaError::Category FormulaError::GetCategory() const {
    return category_;
}

bool FormulaError::operator==(FormulaError rhs) const {
    return category_ == rhs.category_;
}

std::string_view FormulaError::ToString() const {
    switch (category_) {
    case Category::Ref:
        return "#REF!"sv;
    case Category::Value:
        return "#VALUE!"sv;
    case Category::Arithmetic:
        return "#ARITHM!"sv;
    default:
        return "Unexpected"sv;
    }
}
