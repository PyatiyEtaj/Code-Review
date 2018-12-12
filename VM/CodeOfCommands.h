#ifndef VM_CODEOFCOMMANDS_H
#define VM_CODEOFCOMMANDS_H

#include <cstdint>

enum ProcessorCommands : uint8_t{
    STOP = 0,
    SUMI = 1,
    SUMU = 2,
    SUMF = 3,
    SUBI = 4,
    SUBU = 5,
    SUBF = 6,
    MULI = 7,
    MULU = 8,
    MULF = 9,
    DIVI = 10,
    DIVU = 11,
    DIVF = 12,
    MODI = 13,
    MODU = 14,
      OR = 15,
     AND = 16,
     XOR = 17,
     SHIFTLEFT = 18,
    SHIFTRIGHT = 19,
    COMPAREI = 20,
    COMPAREU = 21,
    COMPAREF = 22,
    SUMSHORT = 64,
    SUBSHORT = 65,
    SUMSHORTCONST = 204,
    SUBSHORTCONST = 205,
    SETVALUE = 206,
    MOVE = 66,
    UPLOUDREGS = 201,
    INPUT = 202,
    OUTPUT = 203,
    CALLSUBROUTINE = 245,
    RETURN = 246,
    JUMPUNCOND = 247,
    JUMPUNCONDABOVE = 248,
    JUMPUNCONDEQUAL = 249,
    JUMPUNCONDBELOW = 250,
    JUMPUNCONDABOVEEQAUL = 251,
    JUMPUNCONDNOTEQUAL = 252,
    JUMPUNCONDBELOWEQUAL = 253,

};

enum JumpType : uint8_t {
    Straight = 0,
    DirectIndirect = 1,
    DirectIndirectRegs = 2,
    Relative = 3
};

// зарезервировал 15 регистров для отладочных данных
// 240-243 -- чтение по адресам Memory Read (MemR)
// 244-247 -- запись по адресам Memory Write (MemW)
// 248-251 -- адреса команд Command (CMD)
// 252-255 -- резерв
enum TraceRegs : uint8_t {
    CountOfRegs = 4,
    MemR = 240,
    MemW = 244,
    CMD  = 248
};


#endif //VM_CODEOFCOMMANDS_H
