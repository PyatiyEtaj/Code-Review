#ifndef ASSEMBLER_ASSEMBLER_H
#define ASSEMBLER_ASSEMBLER_H

#include <map>
#include "BinaryFstream.h"
#include "structs.h"
#include "Lexer.h"
#include "CodeOfCommands.h"
#include "AsmCommands.h"

#define ADRTYPE ']'
#define ITSANUM 'n'

//const char ADRTYPE = ']'; // тип адресации

class Assembler{
public:
    // псевдонимы
    using TableEl = std::pair<std::string, uint16_t >; // элемент таблицы | имя - назначение
    using Table   = std::map <std::string, uint16_t >;
    using Name = Lexer::Lexem; // переименовывание лексем
    // типы директив/ошибок
    enum TypeOfD : uint8_t {dStart, dEnd, dTag,  dMov, dAdd, dSub, dMul, dDiv, dMod, dOr, dAnd, dXor, dSLeft, dSRight, dCmp, dAddR, dSubR, dInit, dIn, dOut,
        dUpld, dCall, dJmp, dJmpa, dJmpb, dJmpe, dJmpae, dJmpbe, dJmpne, TagInfo, Operand, Number, Character, EndTag, Miss,
        ErrorWrongEndTag, ErrorSyntax, ErrorName, ErrorCountOfOperand, ErrorDuplicatedTag };
    //********************************
    // исп. при выводе ошибок
    std::vector<std::string> debugV = {"Start", "End", "Tag", "Mov", "Add", "Sub", "Mul", "Div", "Mod", "Or", "And", "Xor",
                                       "SLeft", "SRight", "dCmp", "AddR", "SubR", "Init", "In", "Out", "Upld", "dCall", "dJmp", "dJmpa", "dJmpb",
                                       "dJmpe", "dJmpae", "dJmpbe", "dJmpne", "TagInfo","Operand", "Number", "Character",
                                       "EndTag", "Miss", "Ошибка! Неверное окончание метки-функции", "Ошибка синтаксиса", "Ошибка в имени", "Ошибка в кол-ве операндов",
                                       "Ошибка! Метка с таким именем уже существует"};
    //********************************
    // типы поддерживаемые ассемблером
    enum TypesOfOp : int8_t {tINT, tUINT, tREAL, // тут понятно, что за типы
        tCHAR, tTAG,// CHAR - указывает на тип char, TAG - указывает на метку
        tCONST, // данные типы обозначают константы. Исп. лишь в директивах короткой арифм.
        tERROR = -1};// обозначает ошибочный тип
private:
    LastFreeMemoryAddresses LFMA; // последние свободные ячейки памяти для данных и команд
    std::vector<Name> Lexems; // лексемы, собранные и отсортированные при первом проходе
    Table TagTable;  // таблица имен, заполеняется при первом проходе
    Table RegsTable; // таблица адресных регистров, заполеняется при первом проходе
    std::vector<std::string> Code; // изначальный код программы
    class Command* D[UINT8_MAX];

    bool FirstBypassing(const std::string &FileName) noexcept;  // первый проход
    bool SecondBypassing(const std::string &FileName) noexcept; // второй проход
public:
    Assembler();
    int8_t DefineType(const std::string &Op) const;// определение типа, который передает Op
                                                   // исп. директивами и при первом проходе
    void UpdateLastCOP  (uint16_t Cop); // изменение последней доступной ячейки памяти для команд
    void UpdateLastData (uint16_t Data);// изменение последней доступной ячейки памяти для данных
    bool Run(const std::string &FileName, const std::string &outputBIN) noexcept; // выполнение 1-го и 2-го прохода
    LastFreeMemoryAddresses GetLFMA() const noexcept { return LFMA; };
    Table& GetTagsTable () noexcept { return TagTable; }
    Table& GetRegsTable () noexcept { return RegsTable; }
    void AddReg(const TableEl &el);

    friend std::ostream& operator << (std::ostream& os, const Assembler &a);
    // исключение, класс содержащий некоторое сообщение об ошибке
    using Message = std::string;
};
#endif //ASSEMBLER_ASSEMBLER_H
