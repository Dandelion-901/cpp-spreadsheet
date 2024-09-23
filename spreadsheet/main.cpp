#include <limits>

#include "common.h"
#include "formula.h"
#include "user_interface.h"

#include "tests.h"

int main() {

	setlocale(LC_ALL, "Russian");

	try {
		RunTests();
		std::cout << "\nВсе тесты пройдены успешно, нажми \"Enter\" чтобы продолжить\n";
		std::cin.get();
		system("cls");
	}
	catch (...) {
		std::cout << "\nПройдены не все тесты, завершение программы, нажми \"Enter\" чтобы продолжить\n";
		std::cin.get();
		return 0;
	}

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
