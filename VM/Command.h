#ifndef VM_COMMAND_H
#define VM_COMMAND_H

#include <functional>
#include "processor.h"
#include "CodeOfCommands.h"

class Command{
public:
    virtual void operator ()(class Processor &Proc, const Types &t) = 0;
};
// остановка работы
class Stop : public Command{
public:
    void operator ()(class Processor &Proc, const Types &t) override {}
};
//**********************************************************************************
// Арифметика целая короткая--------------------------------------------------------
class SumShort : public Command{
public:
    void operator ()(class Processor &Proc, const Types &t) override;
};

class SubShort : public Command{
public:
    void operator ()(class Processor &Proc, const Types &t) override;
};
class SumShortConst : public Command{
public:
    void operator ()(class Processor &Proc, const Types &t) override;
};

class SubShortConst : public Command{
public:
    void operator ()(class Processor &Proc, const Types &t) override;
};

//**********************************************************************************
// перемещение --------------------------------------------------------
class Move : public Command{
public:
    void operator ()(class Processor &Proc, const Types &t) override;
};
//**********************************************************************************
class UploadRegs : public Command{
public:
    void operator ()(class Processor &Proc, const Types &t) override;

};
//**********************************************************************************
// ввод/вывод
class Input : public Command{
public:
    void operator ()(class Processor &Proc, const Types &t) override;
};

class Output : public Command{
public:
    void operator ()(class Processor &Proc, const Types &t) override;
};
//**********************************************************************************
// ПЕРЕХОДЫ ------------------------------------------------------------------------
// безусловный переход
class JumpUncond : public Command{
public:
    void operator ()(class Processor &Proc, const Types &t) override;
};
// условные переходы
class JumpCondAbove : public Command{ // проверка >
public:
    void operator ()(class Processor &Proc, const Types &t) override;
};

class JumpCondEqual : public Command{ // проверка ==
public:
    void operator ()(class Processor &Proc, const Types &t) override;
};

class JumpCondBelow : public Command{ // проверка <
public:
    void operator ()(class Processor &Proc, const Types &t) override;
};

class JumpCondAboveEqual : public Command{ // проверка >=
public:
    void operator ()(class Processor &Proc, const Types &t) override;
};

class JumpCondNotEqual : public Command{ // проверка !=
public:
    void operator ()(class Processor &Proc, const Types &t) override;
};

class JumpCondBelowEqual : public Command{ // проверка <=
public:
    void operator ()(class Processor &Proc, const Types &t) override;
};
// вызов подпрограммы
class CallSubroutine : public Command{
public:
    void operator ()(class Processor &Proc, const Types &t) override;
};
// возврат - прямой безусловный переход
class Return : public Command{
public:
    void operator ()(class Processor &Proc, const Types &t) override;
};
// установка значения по адресу
class SetValue : public Command{
public:
    void operator ()(class Processor &Proc, const Types &t) override;
};


#endif //VM_COMMAND_H
