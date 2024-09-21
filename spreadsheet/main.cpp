#include <limits>

#include "common.h"
#include "formula.h"
#include "user_interface.h"

#include "tests.h"

int main() {
	//RunTests();

	setlocale(LC_ALL, "Russian");

	auto sheet = CreateSheet();
	UserInterfece interf(std::cin, std::cout, *sheet);

	while (true) {
		InputData data = interf.Read();
		if (Actions::EXIT == data.action) {
			break;
		}

		interf.Execute(data);
	}
}
