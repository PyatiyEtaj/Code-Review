#include <iostream>
#include <cstring>
#include "processor.h"
#include "Debugger.h"

int main(int argc, char* argv []) {
    if (argc == 2) {
        std::string inputFile = argv[1];
        Processor p;
        int IP = UploaderBIN(p, inputFile);
        if (IP != -1) {
            p.Run(uint16_t (IP));
            std::cout << "Завершение работы!\n";
        }
        else
            std::cerr << "Ошибка стартового адреса!";
    }
    else if (argc == 3)
    {
        std::string inputFile = argv[1];
        std::string log = argv[2];
        Debugger::Run(inputFile, log);
    }
    else
    {
        std::cerr << "Ожидалось: 1 параметр  - имя входного файла (обычное выполнение)\n";
        std::cerr << "           2 параметра - имя входного файла и лог (отладка)\n";
    }
    return 0;
}