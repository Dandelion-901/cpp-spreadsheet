#pragma once

#include "common.h"
#include "formula.h"

#include <iostream>

enum struct Actions {
	BAD_ACTION,
	SET_CELL,
	CLEAR_CELL,
	GET_SCOPE,
	PRINT_VALUE,
	PRINT_TEXT,
	EXIT
};

struct InputData {
	Position pos;
	std::string data;
	Actions action{ Actions::BAD_ACTION };
};

class InputReader {
private:
	std::istream& in_;

public:
	explicit InputReader(std::istream& input);

	InputData Read();
};

class Executor {
private:
	std::ostream& out_;
	SheetInterface& sheet_;

public:
	explicit Executor(std::ostream& output, SheetInterface& sheet);

	void Execute(InputData& data);
};

class UserInterfece final
	: public InputReader
	, public Executor
{
public:
	UserInterfece() = delete;
	UserInterfece(UserInterfece&) = delete;
	explicit UserInterfece(std::istream& input, std::ostream& output, SheetInterface& sheet);

	UserInterfece& operator=(const UserInterfece&) = delete;
	UserInterfece& operator=(UserInterfece&&) = default;

	~UserInterfece() = default;
};
