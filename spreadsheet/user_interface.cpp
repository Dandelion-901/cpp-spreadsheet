#include "user_interface.h"

#include "sstream"

static inline std::ostream& operator<<(std::ostream& output, Position pos) {
    return output << "(" << pos.row << ", " << pos.col << ")";
}

static inline Position operator"" _pos(const char* str, std::size_t) {
    return Position::FromString(str);
}

static inline std::ostream& operator<<(std::ostream& output, Size size) {
    return output << "(" << size.rows << ", " << size.cols << ")";
}

static inline std::ostream& operator<<(std::ostream& output, const CellInterface::Value& value) {
    std::visit(
        [&](const auto& x) {
            output << x;
        },
        value);
    return output;
}

InputReader::InputReader(std::istream& input)
    : in_(input)
{}

InputData InputReader::Read() {
    using namespace std::literals;

    InputData data;

    std::string txt;
    if (in_.good()) {
        std::getline(in_, txt);
    }
    else {
        return data;
    }

    if (txt.empty()) {
        return data;
    }

    std::istringstream txt_stream(txt);
    txt_stream >> txt;

    if ("set"s == txt) {
        if (txt_stream >> txt) {
            data.action = Actions::SET_CELL;
            data.pos = Position::FromString(txt);
        }
    }
    else if ("clear"s == txt) {
        if (txt_stream >> txt) {
            data.action = Actions::CLEAR_CELL;
            data.pos = Position::FromString(txt);
        }
    }
    else if ("scope"s == txt) {
        data.action = Actions::GET_SCOPE;
    }
    else if ("value"s == txt) {
        data.action = Actions::PRINT_VALUE;
    }
    else if ("text"s == txt) {
        data.action = Actions::PRINT_TEXT;
    }
    else if ("exit"s == txt) {
        data.action = Actions::EXIT;
    }

    while (isspace(txt_stream.peek())) {
        txt_stream.get();
    }
    std::getline(txt_stream, data.data);

    return data;
}

Executor::Executor(std::ostream& output, SheetInterface& sheet)
    : out_(output)
    , sheet_(sheet)
{}

void Executor::Execute(InputData & data) {
    using namespace std::literals;
    
    try {
        switch (data.action) {
        case (Actions::BAD_ACTION): {
            out_ << "Неверно введено действие\n"sv;
            break;
        }
        case (Actions::SET_CELL): {
            sheet_.SetCell(data.pos, data.data);
            out_ << "Добавлена ячейка на позицию "sv
                << data.pos.ToString()
                << '\n';
            break;
        }
        case (Actions::CLEAR_CELL): {
            sheet_.ClearCell(data.pos);
            out_ << "Очищенна ячейка на позиции "sv
                << data.pos.ToString()
                << '\n';
            break;
        }
        case (Actions::GET_SCOPE): {
            out_ << "Текущая область видимости таблицы "sv
                << sheet_.GetPrintableSize() << '\n';
            break;
        }
        case (Actions::PRINT_VALUE): {
            system("cls");
            sheet_.PrintValues(out_);
            break;
        }
        case (Actions::PRINT_TEXT): {
            system("cls");
            sheet_.PrintTexts(out_);
            break;
        }
        default:
            throw std::exception("Неопределенное поведение");
        }
    }
    catch (InvalidPositionException) {
        out_ << "Позиция ячейки задана неверно либо выходит за допустимые пределы\n"sv;
    }
}

UserInterfece::UserInterfece(std::istream& input, std::ostream& output, SheetInterface& sheet)
    : InputReader(input)
    , Executor(output, sheet)
{}
