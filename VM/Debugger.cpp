#include "Debugger.h"

std::ofstream OutputLog;

// устанавливаем значение регистра
void SetReg(Processor &cpu) noexcept {
    uint16_t reg, adr;
    std::cout << "Введите номер регистра и значение >> ";
    std::cin >> reg >> adr;
    reg %= TraceRegs::MemR;
    cpu.AddressRegisters[reg] = adr;
}

// просматриваем значения в памяти в опред. диапазоне
std::string CheckMem(class Processor &cpu) noexcept {
    uint16_t f = 0, s = 0;
    uint16_t type = 0;
    std::cout << "Введите диапазон >> ";
    std::cin >> f >> s;
    std::cout << "Введите тип (0-int | 1 - uint | 2 - real) >> "; std::cin >> type;
    if (s < f) std::swap(s,f);
    std::string res = "Значения из диапазона [" + std::to_string(f) + ", " + std::to_string(s) + "] = ";
    for (; f <= s; f++){
        switch (type){
            case 0:
                res += std::to_string(cpu.Memory[f].integer);
                break;
            case 1:
                res += std::to_string(cpu.Memory[f].uinteger);
                break;
            default:
                res += std::to_string(cpu.Memory[f].real);
                break;
        }
        res += " ";
    }
    return res;
}
// выставляем значение для ячейки в памяти
void SetMem(class Processor &cpu) noexcept {
    uint16_t adr, type;
    Types t;
    std::cout << "Введите адрес и тип устанавливаемого значения >> ";
    std::cin >> adr >> type;
    switch (type){
        case 0:
            std::cout << "Введите значение int >> ";
            std::cin >> t.integer;
            break;
        case 1:
            std::cout << "Введите значение uint >> ";
            std::cin >> t.uinteger;
            break;
        default:
            std::cout << "Введите значение real >> ";
            std::cin >> t.real;
            break;
    }
    cpu.Memory[adr] = t;
}
// создаем/изменяем точку остановки
void MakeSP(class Processor &cpu) noexcept {
    uint16_t i,adr;
    std::cout << "Введите номер регистра (1-12) и адрес >> ";
    std::cin >> i >> adr;
    i = (i-1) % (TraceRegs::CountOfRegs*3);
    if (i < TraceRegs::CountOfRegs)
        std::cout << "Создана точка останова по адресу чтения - " << adr << "\n";
    else if (i < TraceRegs::CountOfRegs*2)
        std::cout << "Создана точка останова по адресу записи - " << adr << "\n";
    else
        std::cout << "Создана точка останова для адреса команды - " << adr << "\n";
    cpu.AddressRegisters[TraceRegs::MemR + i] = adr;
}
// удаляем опред. точку остановки
void DeleteSp(class Processor &cpu) noexcept {
    uint16_t i;
    std::cout << "Введите номер удаляемой точки останова (1-12) >> ";
    std::cin >> i;
    i = (i-1) % (TraceRegs::CountOfRegs*3);
    cpu.AddressRegisters[TraceRegs::MemR+i] = Processor::SystemConsts::ZERO;
}
// обнуляем все точки остановок
void DeleteAllSp(class Processor &cpu) noexcept {
    for (int i = TraceRegs::MemR; i < TraceRegs::CMD+TraceRegs::CountOfRegs; i++)
        cpu.AddressRegisters[i] = Processor::SystemConsts::ZERO;
}
// переход к след. точки остановки
void NextSP(class Processor &cpu) noexcept {
    cpu.psw.Flags &= 0xFFFB; // убираем флаг трассировки
}

// переход к след. команде
void SetTraceFlag(class Processor &cpu) noexcept {
    cpu.psw.Flags |= PSW::AllFlags::trace; // выставляем флаг трассировки
}

// пропускаем все точки остановок
void ToEnd(class Processor &cpu) noexcept {
    DeleteAllSp(cpu); // удаляем все точки остановок
    NextSP(cpu);      // убираем флаг трассировки
}

// первоначальный заход в отладчик
// выставление первоначальных точек останова
void InitSP(class Processor &cpu)noexcept {
    std::string cmd;
    std::cout << "********************************** ИНФО ***********************************\n";
    std::cout << "Это начальное меню предназначено для создания первых точек останова\n";
    std::cout << "потому здесь работают лишь команды:\n--make_bp создание точек останова\n";
    std::cout << "--start запуск отладчика\n";
    std::cout << "Доступно 12 регистров для создания точек останова (по 4 на каждый тип)\n";
    std::cout << " 1-4 x  -- установить точку остановки по обращению к адресу памяти x\n";
    std::cout << " 5-8 x  -- установить точку остановки по записи в ячейку памяти x\n";
    std::cout << "9-12 x  -- установить точку остановки по адресу команды x\n";
    std::cout << "***************************************************************************\n";
    while (cmd != "start"){
        std::cout << "Введите команду >> ";
        std::cin >> cmd;
        if (cmd == "make_bp")
            MakeSP(cpu);
        else if (cmd != "start")
            std::cout << "В данном меню разрешены лишь 2 команды make_bp и start\n";
    }
}

// некоторый интерфейс для пользователя
void Interface(class Processor &cpu) noexcept {
    std::string cmd;
    std::cout << "Список команд:\n";
    std::cout << "   make_bp номер_регистра адрес -- создание точки остановки по адресу\n";
    std::cout << "   set_reg номер_регистра адрес -- присвоить регистру адрес\n";
    std::cout << "   set_mem адрес тип значение   -- установить по адресу в памяти значение указанного типа\n";
    std::cout << " check_mem левая_г права_г тип  -- просмотр значений в указанном диапазоне\n";
    std::cout << "    del_bp номер_регистра       -- удаление точки останова\n";
    std::cout << "   del_all -- удаление всех точек остановок\n";
    std::cout << "   next_bp -- переход к след. точке остановки\n";
    std::cout << "   next_cp -- переход к след. вып. команде\n";
    std::cout << "    to_end -- выполнение программы до конца\n";
    std::cout << "     clear -- очистить консоль\n";
    std::cout << "Ввод команды >> \n";
    while (std::cin >> cmd) {
        if (cmd == "make_bp")
            MakeSP(cpu);
        else if (cmd == "set_reg")
            SetReg(cpu);
        else if (cmd == "set_mem")
            SetMem(cpu);
        else if (cmd == "check_mem") {
            auto str = CheckMem(cpu);
            std::cout << str;
            OutputLog << str;
        }
        else if (cmd == "del_bp")
            DeleteSp(cpu);
        else if (cmd == "del_all")
            DeleteAllSp(cpu);
        else if (cmd == "next_bp"){
            NextSP(cpu);
            return;
        }
        else if (cmd == "next_cp") {
            SetTraceFlag(cpu);
            return;
        }
        else if (cmd == "to_end"){
            ToEnd(cpu);
            return;
        }
        else if (cmd == "clear")
            system("clear");
        else
            std::cout << "Команда " << cmd << " отсутствует\n";
        std::cout << "Ввод команды >> \n";
    }

}

// подобие прерывания, в точке где происходит данное прерывание
// процессор передает управление отладчику и себя для сбора инф-ции
void Debugger::TraceInterrupt(uint16_t idx, Processor &cpu) {
    auto checkMem = [&](uint16_t pos, uint8_t typeOf) -> std::string{
        std::string res;
        res = "  память[" + std::to_string(idx) + "] ";
        switch (typeOf){
            case 0:
                res += "| int| -- " + std::to_string(cpu.Memory[pos].integer);
                break;
            case 1:
                res += "|uint| -- " + std::to_string(cpu.Memory[pos].uinteger);
                break;
            default:
                res += "|real| -- " + std::to_string(cpu.Memory[pos].real);
                break;
        }
        return res;
    };
    auto output = [&](std::ostream &os) -> void{
        os << "=========< Отладочная информация >=========\n";
        os << "Регистры:\n";
        for (int i = 1; i < TraceRegs::MemR; i++)
            if (cpu.AddressRegisters[i] != Processor::SystemConsts::ZERO)
                os << "  r" << i << " = " << cpu.AddressRegisters[i] << "\n";
        os << "Точки остановок:\n";
        for (int i = TraceRegs::MemR; i < TraceRegs::CMD + TraceRegs::CountOfRegs; i++)
            if (cpu.AddressRegisters[i] != Processor::SystemConsts::ZERO)
                os << "  " << i << " -- "  << std::setfill('0') << std::setw(5) << cpu.AddressRegisters[i] << "\n";
        os << "Остановка по адресу " << std::setfill('0') << std::setw(5) << idx << "\n";
        os << "  значение по адресу: ";
        if (idx <= LastFreeMemoryAddresses::Limits::CommandL){
            if ((cpu.Memory[idx].CMD1.COP < 200) || (cpu.Memory[idx].CMD1.COP > 244 && cpu.Memory[idx].CMD1.r1 == 2)){
                os << " |КМД тип 1| -- ";
                cpu.Memory[idx].CMD1.OutputDebugInfo(os);
            }
            else{
                os << " |КМД тип 2| -- ";
                cpu.Memory[idx].CMD2.OutputDebugInfo(os);
            }
            os << "\n";
        }
        else
        {   // при срабатывании прерывания, при поптыке чтения или записи, т.к.
            // не известно какой тип данных хранится в памяти решил вывести
            // все три типа
            os << checkMem(idx, 0) << "\n";
            os << checkMem(idx, 1) << "\n";
            os << checkMem(idx, 2) << "\n";
        }
    };
    output(std::cout); // вывод отладочной ин-ции в консоль
    output(OutputLog); // вывод отладочной ин-ции в файл
    Interface(cpu);    // интерфейс для пользователя, выполнение различных команд
}

void Debugger::Run(const std::string &inputFN, const std::string &logName)  {
    OutputLog.open(logName);
    Processor cpu;
    // IP изначально имеет тип инт так как в случае неудачи
    // загрузчик возвращает -1
    int IP = UploaderBIN(cpu, inputFN); // загрузчик
    InitSP(cpu); // устанавливаем первые точки остановок
    std::cout << "-- Начало выполнения --\n";
    std::cout << "начальный адрес = " << uint16_t (IP) << "\n";
    if (IP != -1) {
        // запуск на выполнение процессора
        cpu.Run(uint16_t (IP), true);
        std::cout << "Завершение работы!\n";
    }
    else
        std::cerr << "Ошибка стартового адреса!";
}