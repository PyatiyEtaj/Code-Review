#include "AsmCommands.h"
#include "Interpreter.h"

std::string adr = "a";

// для того чтобы получить номер регистра
// стирается первый символ r, чтобы можно было
// преобразовать из string в uint8_t
// 0-й регистр зарезервирован
inline uint8_t translate(std::string s){
    s[0] = ' ';
    auto n = static_cast<uint8_t>(std::stoi(s));
    // 0-й зарезервирован для адреса возврата из подпрограммы
    // 255 участвует в equ, хранит адрес элемента в памяти
    if (n == 0 || n == 255)
        throw Assembler::Message("Попытка исп. зарезервированный регистр");
    return n;
}
// запись директивы в виде команд виртуал. машины в бин файл
void Command::WriteBF(class Assembler &A, BinaryFstream &bf, Types t)
{
    //записываю команду в бин файл
    bf << std::string("c") << t;
    A.UpdateLastCOP(1); // обновляю последнюю свободную ячейку для команд
}
// запись данных  в бин файл
void Command::WriteBF(class Assembler &A, BinaryFstream &bf, Types t, const std::string &dataType, uint16_t count)
{
    //записываю данные в бин файл
    bf << dataType << t;
    A.UpdateLastData(count); // обновляю последнюю свободную ячейку для данных
}
//*********************************************************************
// присвоение адресу значения
void Init::operator()(class Assembler &A, Command::Operands &Op, BinaryFstream &bf) {
    // опред. тип который нужно установить
    auto T = A.DefineType(Op[0]);
    if (T > Assembler::tCHAR)
        throw Assembler::Message("Доступные типы: int, uint, real");
    Types t;
    // адрес куда необходимо поместить данные
    uint16_t Adr;
    uint8_t Reg;
    std::string TypeOfOperand = "i"; // тип данных
    // вычисляем адрес
    try {
        Adr = std::stoi(Interpreter().Run<uint16_t>(Op[1], A));
    } catch (std::invalid_argument &){
        throw Assembler::Message("Ожидался адрес: " + Op[1]);
    };
    // вычисляем данные
    try {
        switch (T){
            case Assembler::tINT:
                t.integer = std::stoi(Interpreter().Run<int>(Op[2], A));
                break;
            case Assembler::tUINT:
                t.uinteger = std::stoi(Interpreter().Run<uint>(Op[2], A));
                TypeOfOperand = "u";
                break;
            case Assembler::tREAL:
                t.real = std::stoi(Interpreter().Run<float>(Op[2], A));
                TypeOfOperand = "f";
                break;
        }
    }catch (std::invalid_argument &){
        throw Assembler::Message("Ожидался регистр или значение: " + Op[2]);
    };
    try {
        Reg = translate(Op[2]);
    } catch(std::invalid_argument &){
        bf << adr << Adr;
        WriteBF(A, bf, t, TypeOfOperand);
        bf << adr << A.GetLFMA().LastCommand;
        return;
    };
    t.CMD2 = CommandType2(ProcessorCommands::SETVALUE, Reg, Adr);
    WriteBF(A, bf, t);
}
// возвращение из функции или запись окончания работы программы
void End::operator()(class Assembler &A, Command::Operands &Op, BinaryFstream &bf) {
    Types t;
    if (Op[0] == "MAIN")
        t.CMD1 = CommandType1(ProcessorCommands::STOP, 0, 0, 0);
    else
        t.CMD2 = CommandType2(ProcessorCommands::RETURN, 0, 0);
    WriteBF(A, bf, t);
}
void Mov::operator()(class Assembler &A, Command::Operands &Op, BinaryFstream &bf)  {
    uint8_t flag = 0;
    // r2 значение <- r3 значение flag = 0
    // r2 значение <- [r3]адрес   flag = 1
    // [r2]адрес <- r3 значение   flag = 2
    // [r2]адрес <- [r3]адрес     flag = 3
    // выставляю флаги, тип перемещения
    if (*(Op[0].end()-1) == ADRTYPE)
    { flag = 2; Op[0].pop_back();}
    if (*(Op[1].end()-1) == ADRTYPE) {
        if (flag == 2) flag = 3;
        else flag = 1;
        Op[1].pop_back();
    }
    uint8_t Reg1, Reg2;
    try {
        //flg = true;
        Reg1 = translate(Op[0]);
        Reg2 = translate(Op[1]);
    } catch (std::invalid_argument&) {
        throw Assembler::Message("ожидались регистры: " + Op[0] + " " + Op[1]);
    };
    // добавляю регистр в таблицу регистров
    A.AddReg(Assembler::TableEl(Op[0], A.GetRegsTable()[Op[1]]));
    Types t;
    // создаю команду перемещения
    t.CMD1 = CommandType1(ProcessorCommands::MOVE, flag, Reg1, Reg2);
    //записываю команду в бин файл
    WriteBF(A, bf, t);
}

void UpldReg::operator()(class Assembler &A, Command::Operands &Op, BinaryFstream &bf) {
    uint8_t Reg;
    uint16_t Adr;
    try {
        //flg = true;
        Reg = translate(Op[0]);
        Adr = std::stoi(Interpreter().Run<uint16_t>(Op[1], A));
    } catch (std::invalid_argument&) {
        throw Assembler::Message("ожидались регистр и адрес: " + Op[0] + " " + Op[1]);
    };
    // добавляю регистр в таблицу регистров
    A.AddReg(Assembler::TableEl(Op[0], Adr));
    Types t;
    // создаю команду переноса уже готового адреса в регитср
    t.CMD2 = CommandType2(ProcessorCommands::UPLOUDREGS, Reg, Adr);
    //записываю команду в бин файл
    WriteBF(A, bf, t);
}

//ввод
void Input::operator()(class Assembler &A, Command::Operands &Op, BinaryFstream &bf) {
    int TIn = A.DefineType(Op[0]); // определяю тип вводимого значения
    uint8_t Reg;
    // создаю комманду ввода
    Types t;
    t.CMD2 = CommandType2(ProcessorCommands::INPUT, uint8_t(TIn), A.GetLFMA().LastData);
    WriteBF(A, bf, t); // записываю команду в бин.файл
    try {
        Reg = translate(Op[1]);
    }catch (std::invalid_argument&){
        throw Assembler::Message("Ожидался регистр: " + Op[1]);
    };
    // добавляю регистр в таблицу регистров
    A.AddReg(Assembler::TableEl(Op[1], A.GetLFMA().LastData));
    // создается новая команда загрузки регистра
    t.CMD2 = CommandType2(ProcessorCommands::UPLOUDREGS, Reg, A.GetLFMA().LastData);
    WriteBF(A, bf, t); // запись команды в файл
    A.UpdateLastData(1); // увеличиваю номер последней незанятой ячейки данных
}

// вывод
void Output::operator()(class Assembler &A, Command::Operands &Op, BinaryFstream &bf) {
    int TOut = A.DefineType(Op[0]); // определяю тип выводимого значения
    uint8_t Reg;
    try {
        Reg = translate(Op[1]);
    }catch (std::invalid_argument&){
        throw Assembler::Message("Ожидалcя регистр: " + Op[1]);
    };
    // создаю комманду вывода
    Types t;
    t.CMD2 = CommandType2(ProcessorCommands::OUTPUT, uint8_t(TOut), Reg);
    WriteBF(A, bf, t); // записываю команду в бин.файл
}

// метки----------------------------------------------------------------------------------
// макрос - общая часть всех тегов, немного уменьшает код
#define commonPartTag(str, T, unionT) do{               \
    Interpreter interpreter;                            \
    auto lastAdr = A.GetLFMA().LastCommand;             \
    bf << adr << A.GetLFMA().LastData;                  \
    for (auto i = (Op.begin() + 1); i < Op.end(); i++) {\
        Types t;                                        \
        uint16_t r = 1;                                 \
        str;                                            \
        if (*(i->end()-1) == ADRTYPE)                   \
            r = uint16_t (t.unionT)+1;                  \
        WriteBF(A, bf, t, T, r);                        \
    }                                                   \
    bf << adr << lastAdr;                               \
} while(false);                                         \
/////////////////////////////////////////////////////////

void Tag::intTag(class Assembler &A, Command::Operands &Op, BinaryFstream &bf) {
    /*Interpreter interpreter;
    auto lastAdr = A.GetLFMA().LastCommand;
    bf << adr << A.GetLFMA().LastData;
    for (auto i = (Op.begin() + 1); i < Op.end(); i++) {
        Types t;
        uint16_t r = 1;
        t.integer = std::stoi(interpreter.Run<int>(*i, A));
        if (*(i->end()-1) == ADRTYPE) r = uint16_t (t.integer);
        WriteBF(A, bf, t, "i", r);
    }
    bf << adr << lastAdr;*/
    commonPartTag(t.integer = std::stoi(interpreter.Run<int>(*i, A)), "i", integer);
}

void Tag::uintTag(class Assembler &A, Command::Operands &Op, BinaryFstream &bf) {
    /*Interpreter interpreter;
    auto lastAdr = A.GetLFMA().LastCommand;
    bf << adr << A.GetLFMA().LastData;
    for (auto i = (Op.begin() + 1); i < Op.end(); i++) {
        Types t;
        t.uinteger = std::stoul(interpreter.Run<uint>(*i, A));
        WriteBF(A, bf, t, "u");
    }
    bf << adr << lastAdr;*/
    commonPartTag(t.uinteger = std::stoul(interpreter.Run<uint>(*i, A)), "u", uinteger);
}

void Tag::realTag(class Assembler &A, Command::Operands &Op, BinaryFstream &bf) {
    /*Interpreter interpreter;
    auto lastAdr = A.GetLFMA().LastCommand;
    bf << adr << A.GetLFMA().LastData;
    for (auto i = (Op.begin() + 1); i < Op.end(); i++) {
        Types t;
        t.real = std::stof(interpreter.Run<float>(*i, A));
        WriteBF(A, bf, t, "f");
    }
    bf << adr << lastAdr;*/
    commonPartTag(t.real = std::stof(interpreter.Run<float>(*i, A)), "f", real);
}

void Tag::charTag(class Assembler &A, Command::Operands &Op, BinaryFstream &bf) {
    auto lastAdr = A.GetLFMA().LastCommand;
    bf << adr << A.GetLFMA().LastData;
    Types mem;
    try {
        mem.uinteger = std::stoi(Op[1]);
    } catch (std::invalid_argument &) {
        throw Assembler::Message("Ожидался размер массива: " + Op[1]);
    };
    WriteBF(A, bf, mem, "i");
    mem.uinteger *= 4;
    std::string buffer = Op[2];
    while (buffer.size() <= mem.uinteger) buffer.push_back(0);
    for (int j = 0; j < mem.uinteger; j += 4) {
        Types t;
        t.symbols[0] = buffer[j];
        t.symbols[1] = buffer[j + 1];
        t.symbols[2] = buffer[j + 2];
        t.symbols[3] = buffer[j + 3];
        WriteBF(A, bf, t, "s");
    }
    bf << adr << lastAdr;
}


void Tag::operator()(class Assembler &A, Command::Operands &Op, BinaryFstream &bf) {
    // если метка является объявлением
    int TypeOfOp = A.DefineType(Op[0]);
    try {
        switch (TypeOfOp) {
            case (Assembler::tINT):
                intTag(A, Op, bf);
                break;
            case Assembler::tUINT:
                uintTag(A, Op, bf);
                break;
            case Assembler::tREAL:
                realTag(A, Op, bf);
                break;
            case Assembler::tCHAR:
                charTag(A, Op, bf);
                break;
            case Assembler::tTAG: {
                bf << adr;
                bf << A.GetLFMA().LastCommand;
            }
                break;
            default:
                throw Assembler::Message("Ошибочный тип метки: " + Op[0]);
        }
    }catch (std::invalid_argument&){
        throw Assembler::Message("Ошибка в операндах!");
    };
}
// Арифметические операции
// шаблон операций
Types ArithmeticCommand::Func(ArithmeticCommand::Operands &Op, int8_t TypeOfOperands, uint8_t cop) const {
    uint8_t Reg1, Reg2;
    if (TypeOfOperands >= Assembler::TypesOfOp::tCHAR)
        throw Assembler::Message("Для арифметики доступны лишь: int, uint, real!");
    cop += TypeOfOperands;
    // если в команду попали не регистры, бросается исключение
    try {
        Reg1 = translate(Op[1]);
        Reg2 = translate(Op[2]);
    }catch (std::invalid_argument&){
        throw Assembler::Message("ожидались регистры: " + Op[1] + " " + Op[2]);
    };
    Types t;
    t.CMD1 = CommandType1(cop, Reg1, Reg1, Reg2);
    return t;
}
// сложение
void Add::operator()(class Assembler &A, Command::Operands &Op, BinaryFstream &bf) {
    auto TypeOfOperands = A.DefineType(Op[0]); // t = tINT или tUINT и т.д.
    auto WORD = Func(Op, TypeOfOperands, ProcessorCommands::SUMI);
    WriteBF(A, bf, WORD);
}
// вычитание
void Sub::operator()(class Assembler &A, Command::Operands &Op, BinaryFstream &bf) {
    auto TypeOfOperands = A.DefineType(Op[0]); // t = tINT или tUINT и т.д.
    auto WORD = Func(Op, TypeOfOperands, ProcessorCommands::SUBI);
    WriteBF(A, bf, WORD);
}
// умножение
void Mul::operator()(class Assembler &A, Command::Operands &Op, BinaryFstream &bf) {
    auto TypeOfOperands = A.DefineType(Op[0]); // t = tINT или tUINT и т.д.
    auto WORD = Func(Op, TypeOfOperands, ProcessorCommands::MULI);
    WriteBF(A, bf, WORD);
}
// деление
void Div::operator()(class Assembler &A, Command::Operands &Op, BinaryFstream &bf) {
    auto TypeOfOperands = A.DefineType(Op[0]); // t = tINT или tUINT и т.д.
    auto WORD = Func(Op, TypeOfOperands, ProcessorCommands::DIVI);
    WriteBF(A, bf, WORD);
}
// остаток от деления
void Mod::operator()(class Assembler &A, Command::Operands &Op, BinaryFstream &bf) {
    auto TypeOfOperands = A.DefineType(Op[0]); // t = tINT или tUINT и т.д.
    auto WORD = Func(Op, TypeOfOperands, ProcessorCommands::MODI);
    WriteBF(A, bf, WORD);
}
// ИЛИ
void Or::operator()(class Assembler &A, Command::Operands &Op, BinaryFstream &bf) {
    auto WORD = Func(Op, Assembler::TypesOfOp::tINT, ProcessorCommands::OR);
    WriteBF(A, bf, WORD);
}
// И
void And::operator()(class Assembler &A, Command::Operands &Op, BinaryFstream &bf) {
    auto WORD = Func(Op, Assembler::TypesOfOp::tINT, ProcessorCommands::AND);
    WriteBF(A, bf, WORD);
}
// Исключающее ИЛИ
void Xor::operator()(class Assembler &A, Command::Operands &Op, BinaryFstream &bf) {
    auto WORD = Func(Op, Assembler::TypesOfOp::tINT, ProcessorCommands::XOR);
    WriteBF(A, bf, WORD);
}
// сдвиг влево
void SLeft::operator()(class Assembler &A, Command::Operands &Op, BinaryFstream &bf) {
    auto WORD = Func(Op, Assembler::TypesOfOp::tINT, ProcessorCommands::SHIFTLEFT);
    WriteBF(A, bf, WORD);
}
// сдвиг вправо
void SRight::operator()(class Assembler &A, Command::Operands &Op, BinaryFstream &bf) {
    auto WORD = Func(Op, Assembler::TypesOfOp::tINT, ProcessorCommands::SHIFTRIGHT);
    WriteBF(A, bf, WORD);
}

void Cmp::operator()(class Assembler &A, Command::Operands &Op, BinaryFstream &bf) {
    auto TypeOfOperands = A.DefineType(Op[0]);
    auto WORD = Func(Op, TypeOfOperands, ProcessorCommands::COMPAREI);
    WriteBF(A, bf, WORD);
}


Types RegsArithmeticDirective::Func(RegsArithmeticDirective::Operands &Op, uint8_t cop, class Assembler &A, bool isConst) const {
    uint8_t Reg1;
    uint16_t Reg2;
    // isConst -> указывает на тип второго операнда
    // то есть константа или регистр
    try {
        Reg1 = translate(Op[1]);
        if (isConst){
            Reg2 = std::stoi(Interpreter().Run<uint16_t>(Op[2], A));
        }
        else
            Reg2 = translate(Op[2]);
    }catch (std::invalid_argument&){
        throw Assembler::Message("ожидались регистры: " + Op[1] + " " + Op[2]);
    };
    Types t;
    t.CMD1 = CommandType1(cop, Reg1, Reg1, uint8_t(Reg2));
    if (isConst)
        t.CMD2.address = Reg2;
    return t;
}


void AddRegs::operator()(class Assembler &A, Command::Operands &Op, BinaryFstream &bf) {
    uint8_t cop = ProcessorCommands::SUMSHORT;
    bool isConst = false;
    if (A.DefineType(Op[0]) == Assembler::TypesOfOp::tCONST){
        cop = ProcessorCommands :: SUMSHORTCONST;
        isConst = true;
    }
    auto WORD = Func(Op, cop, A, isConst);
    WriteBF(A, bf, WORD);
}

void SubRegs::operator()(class Assembler &A, Command::Operands &Op, BinaryFstream &bf) {
    uint8_t cop = ProcessorCommands::SUBSHORT;
    bool isConst = false;
    if (A.DefineType(Op[0]) == Assembler::TypesOfOp::tCONST){
        cop = ProcessorCommands :: SUBSHORTCONST;
        isConst = true;
    }
    auto WORD = Func(Op, cop, A, isConst);
    WriteBF(A, bf, WORD);
}

void Call::operator()(class Assembler &A, Command::Operands &Op, BinaryFstream &bf) {
    Types t;
    // адрес метки
    uint16_t Adr;
    try { // ищем адрес метки в таблице имен
        Adr = A.GetTagsTable().at(Op[0]);
    }catch (std::out_of_range &){
        // в случае вызова не сущ. ф-ии бросается исключение
        throw Assembler::Message("Вызов не сущ. функции: " + Op[0]);
    }
    // создаем ком. вызова ф-ии
    t.CMD2 = CommandType2(ProcessorCommands::CALLSUBROUTINE, 0, Adr);
    // записываю команду в бин.файл
    WriteBF(A, bf, t);
}

Types JumpCommand::Func(class Assembler &A, Command::Operands &Op, uint8_t cop) const {
    // получение адреса из операнда, который может представляться как готовой константой
    // так и каким-либо выражением, например: A+2, в данном случае A - это метка
    auto f = [&](const std::string &str) -> uint16_t {
        uint16_t Adr;
        try {
            Adr = std::stoi(Interpreter().Run<uint16_t>(str, A));
        } catch (std::invalid_argument&){
            throw Assembler::Message("ожидалась константа: " + str);
        };
        return Adr;
    };
    Types t;
    uint16_t Adr;
    uint8_t Reg1, Reg2;
    // если в команду поступил только один операнд.
    // пример: A - прямой
    // [A] - прямой относительный
    if (Op.size() == 1){
        bool flag = false;
        if (*(Op[0].end()-1) == ADRTYPE) {flag = true; Op[0].pop_back();}
        Adr = f(Op[0]);
        t.CMD2 = CommandType2(cop, JumpType::Straight+flag, Adr);
    } else if (Op.size() == 2){ // если поступили два операнда
        // пример 1: r1, r2 - прямой относительный регистровый
        // пример 2: offset, (A+1)-2 offset указывает на тип перехода-относительный
        if (Op[0] == "offset"){
            Adr = f(Op[1]);
            t.CMD2 =CommandType2(cop, JumpType::Relative, Adr);
        } else{
            try {
                Reg1 = translate(Op[0]);
                Reg2 = translate(Op[1]);
            } catch (std::invalid_argument&){
                throw Assembler::Message("ожидались регистры: " + Op[0] + " " + Op[1]);
            };
            t.CMD1 = CommandType1(cop, JumpType::DirectIndirectRegs, Reg1, Reg2);
        }
    } else
        throw Assembler::Message("Отсутствуют операнды");
    return t;
}

void Jmp::operator()(class Assembler &A, Command::Operands &Op, BinaryFstream &bf) {
    auto WORD = Func(A, Op, ProcessorCommands::JUMPUNCOND);
    WriteBF(A, bf, WORD);
}

void Jmpa::operator()(class Assembler &A, Command::Operands &Op, BinaryFstream &bf) {
    auto WORD = Func(A, Op, ProcessorCommands::JUMPUNCONDABOVE);
    WriteBF(A, bf, WORD);
}

void Jmpe::operator()(class Assembler &A, Command::Operands &Op, BinaryFstream &bf) {
    auto WORD = Func(A, Op, ProcessorCommands::JUMPUNCONDEQUAL);
    WriteBF(A, bf, WORD);
}

void Jmpb::operator()(class Assembler &A, Command::Operands &Op, BinaryFstream &bf) {
    auto WORD = Func(A, Op, ProcessorCommands::JUMPUNCONDBELOW);
    WriteBF(A, bf, WORD);
}

void Jmpae::operator()(class Assembler &A, Command::Operands &Op, BinaryFstream &bf) {
    auto WORD = Func(A, Op, ProcessorCommands::JUMPUNCONDABOVEEQAUL);
    WriteBF(A, bf, WORD);
}

void Jmpne::operator()(class Assembler &A, Command::Operands &Op, BinaryFstream &bf) {
    auto WORD = Func(A, Op, ProcessorCommands::JUMPUNCONDNOTEQUAL);
    WriteBF(A, bf, WORD);
}

void Jmpbe::operator()(class Assembler &A, Command::Operands &Op, BinaryFstream &bf) {
    auto WORD = Func(A, Op, ProcessorCommands::JUMPUNCONDBELOWEQUAL);
    WriteBF(A, bf, WORD);
}

