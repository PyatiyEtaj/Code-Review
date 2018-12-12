#ifndef VM_ARITHEMITICFUNC_H
#define VM_ARITHEMITICFUNC_H

#include <iostream>
#include <limits>
#include "structs.h"
#include "processor.h"

// Арифметика целая длинная-----------------------------------------------------------
class ArithmeticCommand : public Command{
protected:
    template <class T> void SetMem(class Processor &Proc, uint8_t reg, T operand);
    template <class T> T GetFromMem(class Processor &Proc, uint8_t reg);
    template <class T> void SetFlags(class Processor &Proc, const T &t);
public:
    virtual void operator ()(class Processor &Proc, const Types &t) = 0;
};
//***********************************************************
// получение из памяти значений
template<class T> T ArithmeticCommand::GetFromMem(class Processor &Proc, uint8_t reg)
{ return Proc.GetMem(Proc.GetRegs(reg)).real; }

template<> int ArithmeticCommand::GetFromMem(class Processor &Proc, uint8_t reg)
{ return Proc.GetMem(Proc.GetRegs(reg)).integer; }

template<> uint ArithmeticCommand::GetFromMem<uint>(class Processor &Proc, uint8_t reg)
{ return Proc.GetMem(Proc.GetRegs(reg)).uinteger; }
//***********************************************************
//Вставка в память значений
template<class T> void ArithmeticCommand::SetMem(class Processor &Proc, uint8_t reg, T operand)
{
    Types t; t.real = operand;
    Proc.SetMem(Proc.GetRegs(reg), t);
}

template<> void ArithmeticCommand::SetMem<int>(class Processor &Proc, uint8_t reg, int operand)
{
    Types t; t.integer = operand;
    Proc.SetMem(Proc.GetRegs(reg), t);
}

template<> void ArithmeticCommand::SetMem<uint>(class Processor &Proc, uint8_t reg, uint operand)
{
    Types t; t.uinteger = operand;
    Proc.SetMem(Proc.GetRegs(reg), t);
}
//***********************************************************
// установка флагов
template<class T>
void ArithmeticCommand::SetFlags(class Processor &Proc, const T &t){
    uint16_t f(0);
    //T e = T(1.0e-12);
    T e = std::numeric_limits<T>::epsilon();
    if ((t + e) == e) f = PSW::AllFlags::equal;
    else if (t < e) f = PSW::AllFlags::below;
    else if (t > e) f = PSW::AllFlags::above;
    Proc.SetFlags( uint16_t((Proc.GetFlags() & 0xFFFC) | f) );
}
//***********************************************************
// сложение
template <class T>
class Sum : public ArithmeticCommand{
public:
    void operator ()(class Processor &Proc, const Types &t) override{
        T operand = GetFromMem<T>(Proc, t.CMD1.r2) + GetFromMem<T>(Proc, t.CMD1.r3);
        SetFlags<T>(Proc, operand);
        SetMem<T>(Proc, t.CMD1.r1, operand);}
};
//***********************************************************
// вычитание
template <class T>
class Sub : public ArithmeticCommand{
public:
    void operator ()(class Processor &Proc, const Types &t) override{
        T operand = GetFromMem<T>(Proc, t.CMD1.r2) - GetFromMem<T>(Proc, t.CMD1.r3);
        SetFlags<T>(Proc, operand);
        SetMem<T>(Proc, t.CMD1.r1, operand);
    }
};
//***********************************************************
// умножение
template <class T>
class Mul : public ArithmeticCommand{
public:
    void operator ()(class Processor &Proc, const Types &t) override {
        T operand = GetFromMem<T>(Proc, t.CMD1.r2) * GetFromMem<T>(Proc, t.CMD1.r3);
        SetFlags<T>(Proc, operand);
        SetMem<T>(Proc, t.CMD1.r1, operand);
    }
};
//***********************************************************
// деление
template <class T>
class Div : public ArithmeticCommand{
public:
    void operator ()(class Processor &Proc, const Types &t) override{
        // проверка деление на ноль
        auto FABS = [](T value) -> T {
            return(value < 1.0e-12f) ? -value :value;
        };
        T denom = GetFromMem<T>(Proc, t.CMD1.r3), f = GetFromMem<T>(Proc, t.CMD1.r2);
        if (FABS(denom) < 1.0e-12f)
            throw Processor::DivisionByZero();
        //-----------------------------------
        T operand = GetFromMem<T>(Proc, t.CMD1.r2) / denom;
        SetFlags<T>(Proc, operand);
        SetMem<T>(Proc, t.CMD1.r1, operand);
    }
};
//***********************************************************
// Остаток от деления
template <class T>
class Mod : public ArithmeticCommand{
public:
    void operator ()(class Processor &Proc, const Types &t) override{
        T operand = GetFromMem<T>(Proc, t.CMD1.r2) % GetFromMem<T>(Proc, t.CMD1.r3);
        SetFlags<T>(Proc, operand);
        SetMem<T>(Proc, t.CMD1.r1, operand);
    }
};
//***********************************************************
// ИЛИ
template <class T>
class Or : public ArithmeticCommand{
public:
    void operator ()(class Processor &Proc, const Types &t) override{
        T operand = GetFromMem<T>(Proc, t.CMD1.r2) | GetFromMem<T>(Proc, t.CMD1.r3);
        SetFlags<T>(Proc, operand);
        SetMem<T>(Proc, t.CMD1.r1, operand);
    }
};
//***********************************************************
// И
template <class T>
class And : public ArithmeticCommand{
public:
    void operator ()(class Processor &Proc, const Types &t) override{
        T operand = GetFromMem<T>(Proc, t.CMD1.r2) & GetFromMem<T>(Proc, t.CMD1.r3);
        SetFlags<T>(Proc, operand);
        SetMem<T>(Proc, t.CMD1.r1, operand);
    }
};
//***********************************************************
// Исключающее ИЛИ
template <class T>
class Xor : public ArithmeticCommand{
public:
    void operator ()(class Processor &Proc, const Types &t) override{
        T operand = GetFromMem<T>(Proc, t.CMD1.r2) ^ GetFromMem<T>(Proc, t.CMD1.r3);
        SetFlags<T>(Proc, operand);
        SetMem<T>(Proc, t.CMD1.r1, operand);
    }
};
//***********************************************************
// сдвиг влево
template <class T>
class ShiftLeft : public ArithmeticCommand{
public:
    void operator ()(class Processor &Proc, const Types &t) override{
        T operand = GetFromMem<T>(Proc, t.CMD1.r2) << GetFromMem<T>(Proc, t.CMD1.r3);
        SetFlags<T>(Proc, operand);
        SetMem<T>(Proc, t.CMD1.r1, operand);
    }
};
//***********************************************************
// сдвиг вправо
template <class T>
class ShiftRight : public ArithmeticCommand{
public:
    void operator ()(class Processor &Proc, const Types &t) override{
        T operand = GetFromMem<T>(Proc, t.CMD1.r2) >> GetFromMem<T>(Proc, t.CMD1.r3);
        SetFlags<T>(Proc, operand);
        SetMem<T>(Proc, t.CMD1.r1, operand);
    }
};
//***********************************************************
// сравнение
template <class T>
class Compare : public ArithmeticCommand{
public:
    void operator ()(class Processor &Proc, const Types &t) override{
        T operand1 = GetFromMem<T>(Proc, t.CMD1.r2),
                operand2 = GetFromMem<T>(Proc, t.CMD1.r3);
        SetFlags<T>(Proc,operand1-operand2);
    }
};


#endif //VM_ARITHEMITICFUNC_H
