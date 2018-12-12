#ifndef VM_DEBUGGER_H
#define VM_DEBUGGER_H

#include "processor.h"

namespace Debugger{
    // подобие прерывания, в точке где происходит данное прерывание
    // процессор передает управление отладчику и себя для сбора инф-ции
    void TraceInterrupt (uint16_t idx, Processor &cpu);
    // запуск отладчика
    // inputFN - имя входного бинарного файла
    // logName - имя выходного лог файла
    void Run(const std::string &inputFN, const std::string &logName);
};

#endif //VM_DEBUGGER_H
