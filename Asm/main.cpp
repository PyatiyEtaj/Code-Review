#include <iostream>
#include "Assembler.h"
#include "cstring"
#include "AdditionalFunctions.h" // этот функционал необходим лишь для отладки

#define RUN false

int main(int argc, char* argv[]) {
    Assembler A;
#if RUN==true
    bool f = A.Run("input", "inputBIN");
    std::ofstream file("OUT");
    file << A;
    file.close();
    if (f){
        ReadBIN("inputBIN", "OutputBINFile");
        system("./VM inputBIN");
    }
#elif RUN==false
    if (argc > 2){
        std::string input = argv[1];
        std::string outputBIN = argv[2];
        bool f = A.Run(input, outputBIN);
        char cmd [20] = "./VM ";
        strcat(cmd, argv[2]);
        if (f) system(cmd);
    } else {
        std::cerr << "Ожидались имена для входного и выходного файла";
    }
#endif
    return 0;
}