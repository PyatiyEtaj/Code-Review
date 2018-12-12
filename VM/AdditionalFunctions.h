#ifndef VM_ADDITIONALFUNCTIONS_H
#define VM_ADDITIONALFUNCTIONS_H
#include <iostream>
#include <fstream>
#include "processor.h"
#include "BinaryFstream.h"

#define DEBUG false

void CreateBIN(std::string input, std::string output)
{
    std::ifstream f(input);
    BinaryFstream outputF(output);
    uint16_t address = 0;
    if (f.is_open())
    {
        while (!f.eof())
        {
            std::string str, tmp;
            f >> str;
#if DEBUG
            std::cout << str << " ";
#endif
            if (str == "a") // указание адреса ячейки памяти
            {
                // не сделал на прямую outputF << "a " -- возникают проблемы с кодировкой
                tmp = "a "; outputF << tmp;
                f >> address;
                outputF << address;
            }
            else if (str ==  "i") // загрузка в ячейку address значение типа int
            {
                tmp = "i "; outputF << tmp;
                int v; f >> v;
                outputF << v;
            }
            else if (str == "u") // загрузка в ячейку address значение типа uint
            {
                tmp = "u "; outputF << tmp;
                uint v; f >> v;
                outputF << v;
            }
            else if (str ==  "f") // загрузка в ячейку address значение типа real
            {
                tmp = "f "; outputF << tmp;
                float v; f >> v;
                outputF << v;
            }
            else if (str ==  "c")
            {
                int cop(0),r1,r2,r3;
                Types t;
                uint16_t adr;
                f >> cop;f >> r1;
                if ( (cop < 200) || (cop > 244 && r1 == 2))
                {
                    f >> r2; f >> r3;
                    t.CMD1 = CommandType1(uint8_t(cop), uint8_t(r1), uint8_t(r2), uint8_t(r3));
#if DEBUG
                    std::cout << "| " << t.CMD1 << " | ";
#endif
                }
                else
                {
                    f >> adr;
                    t.CMD2 = CommandType2(uint8_t(cop), uint8_t(r1), uint16_t(adr));
#if DEBUG
                    std::cout << "| " << t.CMD2 << " | ";
#endif
                }
                tmp = "c "; outputF << tmp;
                outputF << t;
            }
            else if (str == ";")
            {
                while (str != "\n" || !f.eof())
                    f >> str;
            }
            else if (str == "IP")
            {
                tmp = "IP "; outputF << tmp;
                uint16_t ip; f >> ip;
                outputF << ip;
            }
            else if (str == "END")
            {
                tmp = "END "; outputF << tmp;
                uint16_t end; f >> end;
                outputF << end;
            }
#if DEBUG
            std::cout << "\n";
#endif
        }
    }
}


void ReadBIN(std::string input, std::string output)
{
    std::ofstream outputF(output);
    BinaryFstream f(input, std::ios::binary | std::ios::in);
    uint16_t address = 0;
    if (f.is_open())
    {
        while (!f.eof())
        {
            std::string str;
            f >> str;
            //std::cout << str << "\n";
            if (str == "a") // указание адреса ячейки памяти
            {
                outputF << "a ";
                f >> address;
                outputF << address;
            }
            else if (str ==  "i") // загрузка в ячейку address значение типа int
            {
                outputF << "i ";
                int v; f >> v;
                outputF << v;
            }
            else if (str == "u") // загрузка в ячейку address значение типа uint
            {
                outputF << "u ";
                uint v; f >> v;
                outputF << v;
            }
            else if (str ==  "f") // загрузка в ячейку address значение типа real
            {
                outputF << "f ";
                float v; f >> v;
                outputF << v;
            }
            else if (str ==  "c")
            {
                Types t;
                f >> t;
                outputF << "c ";
                if ( (t.CMD1.COP < 200) || (t.CMD1.COP > 244 && t.CMD1.r1 == 2))
                    outputF << t.CMD1;
                else
                    outputF << t.CMD2;
            }
            else if (str == "IP")
            {
                outputF << "IP ";
                uint16_t ip; f >> ip;
                outputF << ip;
            }
            else if (str == "END")
            {
                outputF << "END ";
                uint16_t end; f >> end;
                outputF << end;
            }
            outputF << "\n";
        }
    }
    f.close();
    outputF.close();
}

#endif //VM_ADDITIONALFUNCTIONS_H
