#ifndef VM_STRUCTS_H
#define VM_STRUCTS_H

#include <iostream>
#include <iomanip>

#define FULL_OUT false

struct PSW{
    enum AllFlags: uint8_t {equal = 0x0000, below = 0x0001, above = 0x0002, trace = 0x0004};
    uint16_t IP;
    uint16_t Flags;
    PSW(): IP(0), Flags(0) {}
};

struct CommandType1{
    uint8_t COP; // КОП
    uint8_t r1,r2,r3; // регистры
    CommandType1(uint8_t COP_, uint8_t r1_,uint8_t r2_,uint8_t r3_)
                    :COP(COP_), r1(r1_), r2(r2_), r3(r3_){};
    // вывод отладочной ин-ции
    void OutputDebugInfo(std::ostream& os) const  noexcept{
        os << "КОП-" << std::setfill('0') << std::setw(3) << int(COP)
           << " р1-" << std::setfill('0')  << std::setw(3) << int(r1)
           << " р2-" << std::setfill('0')  << std::setw(3) << int(r2)
           << " р3-" << std::setfill('0')  << std::setw(3) << int(r3) << "\n";
    }
    // исп. при записи в бин. файл
    friend std::ostream&operator << (std::ostream& os, const CommandType1 &cs){
        os << int(cs.COP) << " " << int(cs.r1) << " " << int(cs.r2) << " " << int(cs.r3);
        return os;
    }
};

struct CommandType2{
    uint8_t  COP; // КОП
    uint8_t  r1;  // регистр
    uint16_t address; // адрес
    CommandType2(uint8_t COP_, uint8_t r1_,uint16_t adr)
                    :COP(COP_), r1(r1_), address(adr){};
    // вывод отладочной ин-ции
    void OutputDebugInfo(std::ostream& os) const  noexcept{
        os << "КОП-"    << std::setfill('0')  << std::setw(3) << int(COP)
           << " р1-"    << std::setfill('0')  << std::setw(3) << int(r1)
           << " адрес-" << std::setfill('0')  << std::setw(5) << address  << "\n";
    }
    // исп. при записи в бин. файл
    friend std::ostream&operator << (std::ostream& os, const CommandType2 &lr){
        os << int(lr.COP) << " " << int(lr.r1) << " " << lr.address;
        return os;
    }
};

struct LastFreeMemoryAddresses{
    enum Limits : uint16_t {CommandL = 32000, DataL = 65000};
    uint16_t LastData;  // последний свободный адрес для данных
    uint16_t LastCommand;   // последний свободный адрес для размещения самой программы
    LastFreeMemoryAddresses():LastCommand(0),        // для команд выделены ячейки [0, 32000]
                              LastData(Limits::CommandL+1){};      // для данных выделены ячейки [32001, 65000]
};


union Types{
    int      integer;
    unsigned uinteger;
    float    real;
    char     symbols[4]; // 4 каких-либо символа
    CommandType1 CMD1;
    CommandType2 CMD2;
    Types(){
        integer = 0;
    };
};

#endif //VM_STRUCTS_H
