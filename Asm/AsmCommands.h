#ifndef ASSEMBLER_DIRECTIVES_H
#define ASSEMBLER_DIRECTIVES_H

#include <vector>
#include "Assembler.h"

class Command{
protected:
    using Operands = std::vector<std::string>;
    // запись в bf, юниона t и обновление последней доступной ячейки для команды на 1
    void WriteBF(class Assembler &A, BinaryFstream &bf, Types t);
    // запись в bf, юниона t и обзначение какой-тип данных был записан dataType
    // и обновление последней доступной ячейки для данных на сount единиц
    void WriteBF(class Assembler &A, BinaryFstream &bf, Types t, const std::string &dataType,  uint16_t count = 1);
public:
    virtual void operator ()(class Assembler &A, Operands &Op, BinaryFstream &bf) = 0;
};
// присвоение адресу значения
class Init : public Command{
public:
    void operator ()(class Assembler &A, Operands &Op, BinaryFstream &bf) override;
};
// установка конца метки
class End : public Command{
public:
    void operator ()(class Assembler &A, Operands &Op, BinaryFstream &bf) override;
};

// Загрузка адресного регистра
class UpldReg : public Command{
public:
    void operator ()(class Assembler &A, Operands &Op, BinaryFstream &bf) override;
};

// перемещение
class Mov : public Command{
public:
    void operator ()(class Assembler &A, Operands &Op, BinaryFstream &bf) override;
};
// вызов ф-ии
class Call : public Command{
public:
    void operator ()(class Assembler &A, Operands &Op, BinaryFstream &bf) override;
};
// переходы
class JumpCommand : public Command {
protected:
    // шаблон выполнения переходов
    // в кач. операндов поступают адреса, метки или регистры
    Types Func(class Assembler &A,Operands &Op, uint8_t cop) const;
public:
    virtual void operator ()(class Assembler &A, Operands &Op, BinaryFstream &bf) = 0;
};

class Jmp : public JumpCommand{
public:
    void operator ()(class Assembler &A, Operands &Op, BinaryFstream &bf) override;
};
// переход с условием >
class Jmpa : public JumpCommand{
public:
    void operator ()(class Assembler &A, Operands &Op, BinaryFstream &bf) override;
};
// переход с условием ==
class Jmpe : public JumpCommand{
public:
    void operator ()(class Assembler &A, Operands &Op, BinaryFstream &bf) override;
};
// переход с условием <
class Jmpb : public JumpCommand{
public:
    void operator ()(class Assembler &A, Operands &Op, BinaryFstream &bf) override;
};
// переход с условием >=
class Jmpae : public JumpCommand{
public:
    void operator ()(class Assembler &A, Operands &Op, BinaryFstream &bf) override;
};
// переход с условием !=
class Jmpne : public JumpCommand{
public:
    void operator ()(class Assembler &A, Operands &Op, BinaryFstream &bf) override;
};
// переход с условием <=
class Jmpbe : public JumpCommand{
public:
    void operator ()(class Assembler &A, Operands &Op, BinaryFstream &bf) override;
};
//---------------------------

class RegsArithmeticDirective : public Command {
protected:
    Types Func(Operands &Op, uint8_t cop, class Assembler &A, bool isConst = false) const;
public:
    virtual void operator ()(class Assembler &A, Operands &Op, BinaryFstream &bf) = 0;
};

// сложение
class AddRegs : public RegsArithmeticDirective{
public:
    void operator ()(class Assembler &A, Operands &Op, BinaryFstream &bf) override;
};
// вычитание
class SubRegs : public RegsArithmeticDirective{
public:
    void operator ()(class Assembler &A, Operands &Op, BinaryFstream &bf) override;
};

// арифметические операции
class ArithmeticCommand : public Command {
protected:
    Types Func(Operands &Op, int8_t TypeOfOperands, uint8_t cop) const;
public:
    virtual void operator ()(class Assembler &A, Operands &Op, BinaryFstream &bf) = 0;
};

// сложение
class Add : public ArithmeticCommand{
public:
    void operator ()(class Assembler &A, Operands &Op, BinaryFstream &bf) override;
};
// вычитание
class Sub : public ArithmeticCommand{
public:
    void operator ()(class Assembler &A, Operands &Op, BinaryFstream &bf) override;
};
// умножение
class Mul : public ArithmeticCommand{
public:
    void operator ()(class Assembler &A, Operands &Op, BinaryFstream &bf) override;
};
// деление
class Div : public ArithmeticCommand{
public:
    void operator ()(class Assembler &A, Operands &Op, BinaryFstream &bf) override;
};
// остаток от деления
class Mod : public ArithmeticCommand{
public:
    void operator ()(class Assembler &A, Operands &Op, BinaryFstream &bf) override;
};
// ИЛИ
class Or : public ArithmeticCommand{
public:
    void operator ()(class Assembler &A, Operands &Op, BinaryFstream &bf) override;
};
// И
class And : public ArithmeticCommand{
public:
    void operator ()(class Assembler &A, Operands &Op, BinaryFstream &bf) override;
};
// исключающее ИЛИ
class Xor : public ArithmeticCommand{
public:
    void operator ()(class Assembler &A, Operands &Op, BinaryFstream &bf) override;
};
// сдвиг влево
class SLeft : public ArithmeticCommand{
public:
    void operator ()(class Assembler &A, Operands &Op, BinaryFstream &bf) override;
};
// сдивг вправо
class SRight : public ArithmeticCommand{
public:
    void operator ()(class Assembler &A, Operands &Op, BinaryFstream &bf) override;
};
// сравнение
class Cmp : public ArithmeticCommand{
public:
    void operator ()(class Assembler &A, Operands &Op, BinaryFstream &bf) override;
};
// метки----------------------------------------------------------------------------------
class Tag : public Command{
private:
    void  intTag(class Assembler &A, Operands &Op, BinaryFstream &bf);
    void uintTag(class Assembler &A, Operands &Op, BinaryFstream &bf);
    void realTag(class Assembler &A, Operands &Op, BinaryFstream &bf);
    void charTag(class Assembler &A, Operands &Op, BinaryFstream &bf);
public:
    void operator ()(class Assembler &A, Operands &Op, BinaryFstream &bf) override;
};
// Ввод/вывод------------------------------------------------------------------------------
class Input : public Command{
public:
    void operator ()(class Assembler &A, Operands &Op, BinaryFstream &bf) override;
};

class Output : public Command{
public:
    void operator ()(class Assembler &A, Operands &Op, BinaryFstream &bf) override;
};

#endif //ASSEMBLER_DIRECTIVES_H
