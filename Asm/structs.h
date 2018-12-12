#ifndef VM_STRUCTS_H
#define VM_STRUCTS_H
#include <iostream>

#define FULL_OUT false
// использую файл из проекта с виртуальной машиной
struct PSW{
    uint16_t IP;
    uint16_t Flags;
    PSW(): IP(0), Flags(0) {}
};

struct CommandType1{
    uint8_t COP; // КОП
    uint8_t r1,r2,r3; // регистры
    CommandType1(uint8_t COP_, uint8_t r1_,uint8_t r2_,uint8_t r3_)
            :COP(COP_), r1(r1_), r2(r2_), r3(r3_){};
    friend std::ostream&operator << (std::ostream& os, const CommandType1 &cs){
#if FULL_OUT
        os << "COP-" << int(cs.COP) << " r1-" << int(cs.r1) << " r2-" << int(cs.r2) << " r3-" << int(cs.r3) << "\n";
#elif !FULL_OUT
        os << int(cs.COP) << " " << int(cs.r1) << " " << int(cs.r2) << " " << int(cs.r3);
#endif
        return os;
    }
};

struct CommandType2{
    uint8_t  COP; // КОП
    uint8_t  r1;  // регистр
    uint16_t address; // адрес
    CommandType2(uint8_t COP_, uint8_t r1_,uint16_t adr)
            :COP(COP_), r1(r1_), address(adr){};
    friend std::ostream&operator << (std::ostream& os, const CommandType2 &lr){
#if FULL_OUT
        os << "COP-" << int(lr.COP) << " r1-" << int(lr.r1) << " address-" << lr.address  << "\n";
#elif !FULL_OUT
        os << int(lr.COP) << " " << int(lr.r1) << " " << lr.address;
#endif
        return os;
    }
};

union Types{
    int      integer;
    unsigned uinteger;
    float    real;
    char     symbols[4]; // 4 каких-либо символа
    CommandType1 CMD1;
    CommandType2 CMD2;
    Types(){
        // некоторые стандартные символы которые можно
        // использовать для форматирования выходных данных
        symbols[0] = '\n';
        symbols[1] = ' ';
        symbols[2] = '\t';
        symbols[3] = '\0';
    };
};

struct LastFreeMemoryAddresses{
    enum Limits : uint16_t {CommandL = 32000, DataL = 65000};
    uint16_t LastData;  // последний свободный адрес для данных
    uint16_t LastCommand;   // последний свободный адрес для размещения самой программы
    LastFreeMemoryAddresses():LastCommand(0),        // для команд выделены ячейки [0, 32000]
                              LastData(Limits::CommandL+1){};      // для данных выделены ячейки [32001, 65000]
};



#endif //VM_STRUCTS_H
