#include <fstream>
#include <iomanip>
#include <bitset>
#include "processor.h"
#include "ArithemiticFunc.h" // Арифметические функции
#include "CodeOfCommands.h"
#include "Debugger.h"

#define DEBUG false // отладочная ин-ция из загрузчиков

Processor::Processor() noexcept {
    // заполняю регистры, недостижимым значением
    // для того чтобы не выводить все регистры при отладке
    // так как их слишком много
    AddressRegisters.assign(SystemConsts::Bit8, ZERO);

    cmd[STOP] = new Stop();
    // Арифметика целая длинная
    cmd[SUMI] = new Sum<int>();
    cmd[SUMU] = new Sum<uint>();
    cmd[SUMF] = new Sum<float>();
    cmd[SUBI] = new Sub<int>();
    cmd[SUBU] = new Sub<uint>();
    cmd[SUBF] = new Sub<float>();
    cmd[MULI] = new Mul<int>();
    cmd[MULU] = new Mul<uint>();
    cmd[MULF] = new Mul<float>();
    cmd[DIVI] = new Div<int>();
    cmd[DIVU] = new Div<uint>();
    cmd[DIVF] = new Div<float>();
    cmd[MODI] = new Mod<int>();
    cmd[MODU] = new Mod<uint>();
    // Арифметика целая длинная - битовые операции
    cmd[ OR]  = new Or<int>();
    cmd[AND]  = new And<int>();
    cmd[XOR]  = new Xor<int>();
    cmd[ SHIFTLEFT]  = new ShiftLeft<int>();
    cmd[SHIFTRIGHT]  = new ShiftRight<int>();
    // сравненение значений
    cmd[COMPAREI]  = new Compare<int>();
    cmd[COMPAREU]  = new Compare<uint>();
    cmd[COMPAREF]  = new Compare<float>();
    // Арифметика целая короткая
    cmd[SUMSHORT]  = new SumShort();
    cmd[SUBSHORT]  = new SubShort();
    cmd[SETVALUE]  = new SetValue();
    cmd[SUMSHORTCONST] = new SumShortConst();
    cmd[SUBSHORTCONST] = new SubShortConst();
    // перемещение
    cmd[MOVE]  = new Move();
    // загрузка адресных регистров
    cmd[UPLOUDREGS] = new UploadRegs();
    // ввод/вывод
    cmd[ INPUT] = new Input();
    cmd[OUTPUT] = new Output();
    // Вызов подпрограммы
    cmd[CALLSUBROUTINE] = new CallSubroutine();
    // возврат прямой переход по адресу, хранящемуся в  r1
    cmd[RETURN] = new Return();
    // безусловный переход
    cmd[JUMPUNCOND] = new JumpUncond();
    // условные переходы
    cmd[JUMPUNCONDABOVE] = new JumpCondAbove();
    cmd[JUMPUNCONDEQUAL] = new JumpCondEqual();
    cmd[JUMPUNCONDBELOW] = new JumpCondBelow();
    cmd[JUMPUNCONDABOVEEQAUL] = new JumpCondAboveEqual();
    cmd[  JUMPUNCONDNOTEQUAL] = new JumpCondNotEqual();
    cmd[JUMPUNCONDBELOWEQUAL] = new JumpCondBelowEqual();
}

// поиск в отладочных регистрах адреса idx
bool Processor::find(uint8_t reg, uint16_t idx) {
    for (int i = 0; i < TraceRegs ::CountOfRegs; i++)
        if (AddressRegisters[reg + i] == idx)
            return true;
    return false;
}

void Processor::Run(uint16_t ip_, bool debug) noexcept {
    DebugMode = debug;
    SetIP(ip_);    // устанавливаем начало работы процессора
    int NumbC = 1; // код операции, если равен 0 -> прекращение работы
    while(NumbC != END)
    {   // для выполнения трасировки после выполнения команды
        bool trace = find(TraceRegs::CMD, psw.IP) || ((psw.Flags & PSW::AllFlags::trace) == PSW::AllFlags::trace);
        uint16_t buf = psw.IP; // буффер, так как комманда может изменить IP, например безусловный переход
        NumbC = Memory[psw.IP].CMD1.COP;
        if (DebugMode)
        {
            try {
                (*cmd[NumbC])(*this, Memory[psw.IP]);
                if (trace) //прерывание по адресу выполняющейся команды
                    Debugger::TraceInterrupt(buf, *this); // подобие прерывания, передача управления отладчику
            }catch (std::out_of_range &e){
                std::cerr << e.what() << "\n";
                return;
            };
        }
        else
        {
            (*cmd[NumbC])(*this, Memory[psw.IP]);
        }
        psw.IP++;
    }
}

void Processor::SetMem(uint16_t idx, const Processor::WORD &w) noexcept {
    Memory[idx] = w;
    // прерывание по записи в ячейку памяти
    if (DebugMode && find(TraceRegs::MemW, idx))
        Debugger::TraceInterrupt(idx, *this);// подобие прерывания, передача управления отладчику
}


Processor::WORD Processor::GetMem(uint16_t idx) noexcept {
    // прерывание по обращению к адресу памяти
    if (DebugMode && find(TraceRegs::MemR, idx))
        Debugger::TraceInterrupt(idx, *this);// подобие прерывания, передача управления отладчику
    return Memory[idx];
}

void Processor::SetRegs(uint8_t idx, uint16_t NewAdr) noexcept {
    AddressRegisters[idx] = NewAdr;
}

uint16_t Processor::GetRegs(uint8_t idx) const noexcept {
    return AddressRegisters[idx];
}

void Processor::SetIP(uint16_t NewIP) noexcept {
    psw.IP = NewIP;
}

uint16_t Processor::GetIP() const noexcept {
    return psw.IP;
}

void Processor::SetFlags(uint16_t flag) noexcept {
    psw.Flags = flag;
}

uint16_t Processor::GetFlags() const noexcept {
    return psw.Flags;
}

void Processor::StackPush(const uint16_t &adr) {
    StackPointer++;
    if (StackPointer < SystemConsts::StackPointerStart)
        throw std::out_of_range("Критическая ошибка! Стек вызовов переполнен");
    WORD w;
    w.uinteger = adr;
    Memory[StackPointer] = w;
}

void Processor::StackPop() {
    StackPointer--;
    if (StackPointer < SystemConsts::StackPointerStart)
        throw std::out_of_range("Критическая ошибка! Стек вызовов пуст");
}

uint16_t Processor::StackTop() const noexcept {
    return static_cast<uint16_t>(Memory[StackPointer].uinteger);
}

// загрузчик (бинарный)
int UploaderBIN(Processor &Proc, const std::string &FileName) {
    BinaryFstream f(FileName, std::ios::binary | std::ios::in);
    uint16_t address = 0;
    int IP = -1; // изначально равен -1, означает, что файл открыть не удалось
    if (f.is_open()) {
        while (!f.eof()) {
            std::string str;
            f >> str;
#if DEBUG
            std::cout << "| str = "<< std::setw(3)<< str << " | ";
#endif
            if (str == "a") // указание адреса ячейки памяти
            {
                f >> address;
                address--;
            } else if (str == "i") // загрузка в ячейку address значение типа int
                f >> Proc.Memory[address].integer;
            else if (str == "u") // загрузка в ячейку address значение типа uint
                f >> Proc.Memory[address].uinteger;
            else if (str == "f") // загрузка в ячейку address значение типа real
                f >> Proc.Memory[address].real;
            else if (str == "s"){
                f >> Proc.Memory[address].symbols[0];
                f >> Proc.Memory[address].symbols[1];
                f >> Proc.Memory[address].symbols[2];
                f >> Proc.Memory[address].symbols[3];
            }
            else if (str == "c") {
                Types t;
                f >> t;
                if ( (t.CMD1.COP < 200) || (t.CMD1.COP > 244 && t.CMD1.r1 == 2)) {
                    Proc.Memory[address].CMD1 = t.CMD1;
#if DEBUG
                    std::cout << "| c = " << std::setw(3) << t.CMD1 << " | ";
#endif
                } else {
#if DEBUG
                    std::cout << "| c = " << std::setw(3) << t.CMD2 << " | ";
#endif
                    Proc.Memory[address].CMD2 = t.CMD2;
                }
            } else if (str == "IP")
                f >> IP; // устанавливаем IP
#if DEBUG
            std::cout<< "| address - " << std::setw(5) <<address << " |\n";
#endif
            address++;

        }
        std::cout << "Чтение " << FileName << " завершено!\n";
    }
    return IP; // файл не удалось открыть
}
