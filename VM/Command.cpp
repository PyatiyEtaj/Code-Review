#include "Command.h"

//******************************************************************************
// загрузка адресных регистров
void UploadRegs::operator()(class Processor &Proc, const Types &t) {
    //std::cout << "| UPLDREG = " << int(t.CMD2.r1) << " " << Proc.GetMem(t.CMD2.address).uinteger << " |";
    Proc.SetRegs(t.CMD2.r1, t.CMD2.address);
}
// вывод
void Output::operator()(class Processor &Proc, const Types &t) {
    CommandType2 CMD2 = t.CMD2;
    switch (CMD2.r1)
    {
        case 0:
            std::cout << Proc.GetMem(Proc.GetRegs(CMD2.address)).integer;
            break;
        case 1:
            std::cout << Proc.GetMem(Proc.GetRegs(CMD2.address)).uinteger;
            break;
        case 2:
            std::cout << Proc.GetMem(Proc.GetRegs(CMD2.address)).real;
            break;
        case 3:
            std::cout << Proc.GetMem(Proc.GetRegs(CMD2.address)).symbols;
            break;
        default:
            std::cout << Proc.GetMem(Proc.GetRegs(CMD2.address)).integer;
            break;
    }
}
// ввод
void Input::operator()(class Processor &Proc, const Types &t) {
    CommandType2 CMD2 = t.CMD2;
    Types tmp;
    switch (CMD2.r1)
    {
        case 0:
            std::cin >> tmp.integer;
            break;
        case 1:
            std::cin >> tmp.uinteger;
            break;
        case 2:
            std::cin >> tmp.real;
            break;
        default:
            std::cin >> tmp.integer;
            break;
    }
    Proc.SetMem(CMD2.address, tmp);
}

//**********************************************************************************
// Арифметика целая короткая--------------------------------------------------------
void SumShort::operator()(class Processor &Proc, const Types &t) {
    uint16_t R2 = Proc.GetRegs(t.CMD1.r2),
             R3 = Proc.GetRegs(t.CMD1.r3);
    Proc.SetRegs(t.CMD1.r1, R2+R3);
}

void SubShort::operator()(class Processor &Proc, const Types &t) {
    uint16_t R2 = Proc.GetRegs(t.CMD1.r2),
             R3 = Proc.GetRegs(t.CMD1.r3);
    Proc.SetRegs(t.CMD1.r1, R2-R3);
}
void SumShortConst::operator()(class Processor &Proc, const Types &t) {
    uint16_t R1 = Proc.GetRegs(t.CMD2.r1) + t.CMD2.address;
    Proc.SetRegs(t.CMD1.r1, R1);
}
void SubShortConst::operator()(class Processor &Proc, const Types &t) {
    uint16_t R1 = Proc.GetRegs(t.CMD2.r1) - t.CMD2.address;
    Proc.SetRegs(t.CMD1.r1, R1);
}
//**********************************************************************************
// перемещение ---------------------------------------------------------------------
void Move::operator()(class Processor &Proc, const Types &t) {
    Types tmp;
    switch (t.CMD1.r1)
    {
        case 0: // r2 значение <- r3 значение
            Proc.SetMem(Proc.GetRegs(t.CMD1.r2), Proc.GetMem(Proc.GetRegs(t.CMD1.r3)));
            break;
        case 1: // r2 значение <- [r3]адрес
            tmp.uinteger = Proc.GetRegs(t.CMD1.r3);
            Proc.SetMem(Proc.GetRegs(t.CMD1.r2), tmp);
            break;
        case 2: // [r2]адрес <- r3 значение
            Proc.SetRegs(t.CMD1.r2, uint16_t (Proc.GetMem(t.CMD1.r3).uinteger));
            break;
        case 3: // [r2]адрес <- [r3]адрес
            Proc.SetRegs(t.CMD1.r2, Proc.GetRegs(t.CMD1.r3));
            break;
        default:
            break;
    }
}
//**********************************************************************************
// безусловный переход
void JumpUncond::operator()(class Processor &Proc, const Types &t){
    uint16_t tmp;
    switch (t.CMD2.r1) // значение r1
    { // везде отнимается единица, так как после выполнения каждой команды IP увеливается на 1
        case JumpType::Straight: // Безусловный прямой
            tmp = uint16_t (t.CMD2.address - 1);
            break;
        case JumpType::DirectIndirect: // Безусловный прямой  косвенный
            tmp = uint16_t (Proc.GetMem(t.CMD2.address).uinteger - 1);
            break;
        case JumpType::DirectIndirectRegs: // Безусловный прямой косвенный регистровый
            tmp = uint16_t(Proc.GetRegs(t.CMD1.r2) + Proc.GetRegs(t.CMD1.r3) - 1);
            break;
        case JumpType::Relative: // Безусловный относительный
            tmp = uint16_t (Proc.GetIP()+ t.CMD2.address - 1);
            break;
        default:
            return;
    }
    Proc.SetIP(tmp);
}
//**********************************************************************************
// условные переходы
// проверка >
void JumpCondAbove::operator()(class Processor &Proc, const Types &t){
    uint16_t flag = Proc.GetFlags();
    if ((flag & PSW::AllFlags::above) == PSW::AllFlags::above)
        JumpUncond()(Proc,t);
}
// проверка ==
void JumpCondEqual::operator()(class Processor &Proc, const Types &t) {
    uint16_t flag = Proc.GetFlags();
    if ((flag | PSW::AllFlags::equal) == PSW::AllFlags::equal)
        JumpUncond()(Proc,t);
}
// проверка <
void JumpCondBelow::operator()(class Processor &Proc, const Types &t) {
    uint16_t flag = Proc.GetFlags();
    if ((flag & PSW::AllFlags::below) == PSW::AllFlags::below)
        JumpUncond()(Proc,t);
}
// проверка >=
void JumpCondAboveEqual::operator()(class Processor &Proc, const Types &t) {
    uint16_t flag = Proc.GetFlags();
    if ((flag & PSW::AllFlags::above) == PSW::AllFlags::above ||
        (flag & PSW::AllFlags::equal) == PSW::AllFlags::equal   )
        JumpUncond()(Proc,t);
}
// проверка !=
void JumpCondNotEqual::operator()(class Processor &Proc, const Types &t) {
    uint16_t flag = Proc.GetFlags();
    if ((flag | PSW::AllFlags::equal) != PSW::AllFlags::equal)
        JumpUncond()(Proc,t);
}
// проверка <=
void JumpCondBelowEqual::operator()(class Processor &Proc, const Types &t) {
    uint16_t flag = Proc.GetFlags();
    if ((flag & PSW::AllFlags::below) == PSW::AllFlags::below ||
        (flag & PSW::AllFlags::equal) == PSW::AllFlags::equal   )
        JumpUncond()(Proc,t);

}

void CallSubroutine::operator()(class Processor &Proc, const Types &t) {
    Proc.StackPush(Proc.GetRegs(t.CMD2.r1));
    Proc.SetRegs(t.CMD2.r1, Proc.GetIP());
    Proc.SetIP(t.CMD2.address - 1);
}


void Return::operator()(class Processor &Proc, const Types &t) {
    Proc.SetIP(Proc.GetRegs(t.CMD2.r1));
    Proc.SetRegs(t.CMD2.r1, Proc.StackTop());
    Proc.StackPop();
}

void SetValue::operator()(class Processor &Proc, const Types &t) {
    Proc.SetMem(t.CMD2.address, Proc.GetMem(Proc.GetRegs(t.CMD2.r1)));
}
