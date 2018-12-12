#ifndef VM_PROCESSOR_H
#define VM_PROCESSOR_H

#include <iostream>
#include <vector>
#include "structs.h" // | Union - Types | Struct - PSW, CommandType1, CommandType2
#include "Command.h" // команды процессора
#include "BinaryFstream.h"   // Интерфейс для работы с бинарными файлами


class Processor {
public:
    enum SystemConsts : uint {ZERO = 65535, Bit8 = 256, Bit16 = 65536, SizeOfStack = 535, StackPointerStart = Bit16-SizeOfStack};
private:
    using WORD = Types;
    const uint8_t END = 0; // так как имя Stop занято командой, назвал END
    class Command* cmd[SystemConsts::Bit8];
    // стек вызовов находится в памяти, первая ячейка стека 65001, доступно для исп. 535 ячеек
    // т.е. можно вызвать не более 535 подпрограмм, далее бросается исключение
    // переполнение стека вызовов
    uint16_t StackPointer = SystemConsts::StackPointerStart;
    // для отладчика
    bool find(uint8_t reg, uint16_t idx); // поиск среди отладочных регистров адресов, на которые были поставлены ловушки
    bool DebugMode = false;
public:
    std::vector<uint16_t> AddressRegisters; // 256 регистров
    WORD Memory[SystemConsts::Bit16];
    PSW psw;

    Processor() noexcept;
    void Run         (uint16_t ip_, bool debug = false) noexcept;

    void     SetMem   (uint16_t idx, const WORD &w) noexcept;
    WORD     GetMem   (uint16_t idx) noexcept;
    void     SetRegs  (uint8_t idx, uint16_t NewAdr) noexcept;
    uint16_t GetRegs  (uint8_t idx) const noexcept;
    void     SetIP    (uint16_t NewIP) noexcept;
    uint16_t GetIP    () const noexcept;
    void     SetFlags (uint16_t flag) noexcept;
    uint16_t GetFlags () const noexcept;
    uint16_t StackTop () const noexcept;
    void     StackPop ();
    void     StackPush(const uint16_t &adr);

    friend int UploaderBIN(Processor &Proc, const std::string &FileName);

    class DivisionByZero{};

};


#endif //VM_PROCESSOR_H
