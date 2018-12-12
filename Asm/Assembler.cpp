#include <functional>
#include <chrono>
#include "Assembler.h"

Assembler::Assembler() {
    D[Assembler::  dMov] = new Mov();
    D[Assembler::  dAdd] = new AddRegs();
    D[Assembler::  dSub] = new SubRegs();
    D[Assembler::  dAdd] = new Add();
    D[Assembler::  dSub] = new Sub();
    D[Assembler::  dMul] = new Mul();
    D[Assembler::  dDiv] = new Div();
    D[Assembler::  dMod] = new Mod();
    D[Assembler::   dOr] = new Or();
    D[Assembler::  dXor] = new Xor();
    D[Assembler::  dAnd] = new And();
    D[Assembler::  dTag] = new Tag();
    D[Assembler::   dIn] = new Input();
    D[Assembler::  dOut] = new Output();
    D[Assembler::dSLeft] = new SLeft();
    D[Assembler::dSLeft] = new SRight();
    //D[Assembler::dStart] = new Start();
    D[Assembler::  dEnd] = new End();
    D[Assembler:: dUpld] = new UpldReg();
    D[Assembler:: dCall] = new Call();
    D[Assembler::  dJmp] = new Jmp();
    D[Assembler:: dJmpa] = new Jmpa();
    D[Assembler::dJmpae] = new Jmpae();
    D[Assembler:: dJmpb] = new Jmpb();
    D[Assembler::dJmpbe] = new Jmpbe();
    D[Assembler:: dJmpe] = new Jmpe();
    D[Assembler::dJmpne] = new Jmpne();
    D[Assembler::  dCmp] = new Cmp();
    D[Assembler:: dInit] = new Init();
    D[Assembler:: dAddR] = new AddRegs();
    D[Assembler:: dSubR] = new SubRegs();
}

void Assembler::UpdateLastCOP(uint16_t Cop) {
    LFMA.LastCommand += Cop;
    if (LFMA.LastCommand > LastFreeMemoryAddresses::CommandL)
        throw Assembler::Message("Достигнуто ограничение по памяти для команд");
}

void Assembler::UpdateLastData(uint16_t Data) {
    LFMA.LastData += Data;
    if (LFMA.LastCommand > LastFreeMemoryAddresses::DataL)
        throw Assembler::Message("Достигнуто ограничение по памяти для данных");
}

// определение типа значения
int8_t Assembler::DefineType(const std::string &Op) const {
    if (Op == "int")
        return TypesOfOp::tINT;
    else if (Op == "uint")
        return TypesOfOp::tUINT;
    else if (Op == "real")
        return TypesOfOp::tREAL;
    else if (Op == "char")
        return TypesOfOp::tCHAR;
    else if (Op == "func")
        return TypesOfOp::tTAG;
    else if (Op == "const")
        return TypesOfOp::tCONST;
    return TypesOfOp::tERROR;
}

bool Assembler::FirstBypassing(const std::string &FileName) noexcept {
    std::vector<Lexer::Lexem> TempLexemes;
    // считывание всех лексем
    Lexer lexer;
    if (!lexer.ReadFile(FileName))
        return false;
    TempLexemes = lexer.GetAllLexems();
    Code = lexer.GetCode();
    //*************************************************************
    // выделение меток
    auto IsTag = [&](size_t &i, const std::vector<Lexer::Lexem> &L) -> Assembler::Name {
        switch (L[i+1].second){ // если после операнда идет : то это метка
            case Lexer::COLON:
            {
                if (L[i + 2].second != Assembler::Operand) // если после : идет какой-либо элемент TypesOfOp, то это точно метка
                    return Assembler::Name(L[i].first, Assembler::dTag, L[i].third);
                if (TagTable.find(L[i].first) != TagTable.end()) // если метка уже существует, ошибка дубликации
                    return Assembler::Name(L[i].first, Assembler::ErrorDuplicatedTag, L[i].third);
                return Assembler::Name(L[i].first, Assembler::ErrorSyntax, L[i].third); // синтаксическая ошибка
                // после метки идет сразу операнд или что-то еще
            }
            default: // иначе это какой-либо операнд
                return Assembler::Name(L[i].first, Assembler::Miss, L[i].third);
        }
    };
    //*************************************************************
    // выделение чисел
    auto IsNumber = [](Name &l) -> void {
        for (auto ch : l.first) // если в строке не присутствуют символы, описанные ниже, то это число
            if (ch == '_' || (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z'))
                return;
        l.second = Assembler::Number;
    };
    //*************************************************************
    // выделение операндов
    auto IsOperand = [&](size_t &i, const std::vector<Lexer::Lexem> &L) -> Assembler::Name {
        std::string str = L[i].first; uint16_t line = L[i].third;
        // обозначение метки
        if (DefineType(L[i].first) != TypesOfOp::tERROR) // если строка не является один из TypesOfOp, то это просто операнд
            return Assembler::Name(str, Assembler::TagInfo, line);
        bool AdressType = false; // тип адресации
        if (i > 0) { // поиск [ и ]
            if (L[i - 1].second == Lexer::LPAR) AdressType = true;
            // прибавляю к операнду все скобки ( и -
            // допустим выражение (((-A + 1)-2)+3)
            // преобразуется в A + 1)-2)+3), что в последствие будет проще обработать
            else if (L[i - 1].second == Lexer::LBRCT || L[i - 1].first == "-"){
                for (auto j = i; j > 0 && (L[j - 1].second == Lexer::LBRCT || L[j - 1].first == "-"); j--)
                    str = L[j - 1].first + str;
            }

        }
        switch (L[i+1].second)
        {   // какой-либо арифметический знак
            case Lexer::RBRCT:
            case Lexer::SIGN: {
                size_t tmp_i = i + 1; // превращаю в один операнд записи типа a+b+c/d и т.п.
                while (L[tmp_i].second != Lexer::COMMA && L[tmp_i].second != Lexer::COMMENT &&
                       L[tmp_i].second != Lexer::END_LINE && L[tmp_i].second != Lexer::ERROR &&
                       L[tmp_i].second != Lexer::RPAR) {
                    i++;
                    tmp_i++;
                    str += L[i].first;

                }// проверка синтаксиса-----------------------------------
                // ошибка с [ и ], недостаточно оперндов
                if ((L[tmp_i].second != Lexer::RPAR &&  AdressType) ||
                    (L[tmp_i].second == Lexer::RPAR && !AdressType) ||
                    (L[i].second == Lexer::SIGN))
                    return Assembler::Name(str, ErrorSyntax, line);
                if (L[tmp_i].second == Lexer::ERROR) // ошибочное имя
                    return Assembler::Name(str, ErrorName, line);
                //---------------------------------------------------------
                if (AdressType) str.push_back(ADRTYPE); // указываю тип адресации
                return Assembler::Name(str, Operand, line);
            }
            // если одиночный операнд
            default:{
                // добавляю все скобки )
                while (L[i+1].second == Lexer::RBRCT) {
                    str += L[i + 1].first; i++;
                }
                // проверка [ и ]
                if ((L[i+1].second != Lexer::RPAR &&  AdressType) ||
                    (L[i+1].second == Lexer::RPAR && !AdressType))
                    return Assembler::Name(str, Assembler::ErrorSyntax, line);
                // после операнда: запятая или конец строки или коммент
                if (L[i+1].second == Lexer::COMMA || L[i+1].second == Lexer::COMMENT ||
                   (L[i+1].second == Lexer::END_LINE) || (L[i+1].second == Lexer::RPAR && AdressType)) {
                    // обозначаю тип адресации: косвенный или прямой
                    if (AdressType) str.push_back(ADRTYPE); // указываю тип адресации
                    return Assembler::Name(str, Assembler::Operand, line);
                }
                return Assembler::Name(str, Assembler::ErrorSyntax, line);
            }
        }
    };
    //*************************************************************
    // ключевые слова
    auto KeyWords = [](size_t &i, const std::vector<Lexer::Lexem> &L) -> Assembler::Name {
        std::string str = L[i].first; uint16_t line = L[i].third;
        uint8_t flg = 0;
        if (str == "mov")    flg = Assembler::dMov;
        if (str == "input")  flg = Assembler::dIn;
        if (str == "output") flg = Assembler::dOut;
        if (str == "add_r")  flg = Assembler::dAddR;
        if (str == "sub_r")  flg = Assembler::dSubR;
        if (str == "add")    flg = Assembler::dAdd;
        if (str == "sub")    flg = Assembler::dSub;
        if (str == "mul")    flg = Assembler::dMul;
        if (str == "div")    flg = Assembler::dDiv;
        if (str == "mod")    flg = Assembler::dMod;
        if (str == "or")     flg = Assembler::dOr;
        if (str == "and")    flg = Assembler::dAnd;
        if (str == "xor")    flg = Assembler::dXor;
        if (str == "sleft")  flg = Assembler::dSLeft;
        if (str == "sright") flg = Assembler::dSRight;
        if (str == "upld")   flg = Assembler::dUpld;
        if (str == "call")   flg = Assembler::dCall;
        if (str == "jmp")    flg = Assembler::dJmp;
        if (str == "jmp_a")  flg = Assembler::dJmpa;
        if (str == "jmp_ae") flg = Assembler::dJmpae;
        if (str == "jmp_b")  flg = Assembler::dJmpb;
        if (str == "jmp_be") flg = Assembler::dJmpbe;
        if (str == "jmp_e")  flg = Assembler::dJmpe;
        if (str == "jmp_ne") flg = Assembler::dJmpne;
        if (str == "cmp")    flg = Assembler::dCmp;
        if (str == "init")   flg = Assembler::dInit;

        return Assembler::Name(str, flg, line);
    };
    //*************************************************************
    // конец метки-функции
    auto EndOfTags = [&](size_t &i, const std::vector<Lexer::Lexem> &L) -> Assembler::Name{
        i++;
        // ключевое слово END указывает на конец метки-функции
        if (TagTable.find(L[i].first) != TagTable.end())
            return Assembler::Name(L[i].first, EndTag, L[i].third);
        // если после END указывается конец несущ. метки, то это ошибка
        return Assembler::Name(L[i-1].first+" "+L[i].first, Assembler::ErrorWrongEndTag, L[i-1].third);
    };
    //*************************************************************
    size_t i = 0;
    Lexer::Lexem l;
    while ( i < TempLexemes.size())
    {
        switch(TempLexemes[i].second)
        {
            case Lexer::IDENT: { // проверка идентификатора
                auto el = IsTag(i, TempLexemes); // является ли меткой
                // если не является меткой, проверка на операнд
                if (el.second == Assembler::Miss) el = IsOperand(i, TempLexemes);
                // если el - не является какой-либо ошибкой
                // проверка операнда -> является ли числом
                if (el.second < Assembler::ErrorWrongEndTag) IsNumber(el);
                // занесение метки в таблицу имен
                if (el.second == Assembler::dTag) {
                    TableEl tmp = TableEl(el.first, 0);
                    TagTable.insert(tmp);
                }
                Lexems.push_back(el);
            }
                break;
            case Lexer::CHARACTER:
                Lexems.emplace_back(TempLexemes[i].first, Assembler::Character, TempLexemes[i].third);
                break;
            case Lexer::KEY_WORDS: // проверка ключевого слова
                Lexems.push_back(KeyWords(i,TempLexemes));
                break;
            case Lexer::END_TAG: // конец метки-функции
                Lexems.push_back(EndOfTags(i, TempLexemes));
                break;
            case Lexer::ERROR: // ошибочное имя
                Lexems.emplace_back(TempLexemes[i].first, Assembler::ErrorName, TempLexemes[i].third);
                break;
            default: // все-возможные случайные знаки игнорируются
                break;
        }
        i++;
    }
    return !(TagTable.find("MAIN") == TagTable.end()); // проверка точки входа
}

bool Assembler::SecondBypassing(const std::string &FileName) noexcept {
    // Создание листинга
    auto Listing = [&](const std::string &FileName) -> void{
        std::ofstream f(FileName+"_listing");
        for (auto el : Lexems) // добавление кода ошибки к строчкам исходного кода
            if (el.second >= Assembler::ErrorWrongEndTag)
                Code[el.third] += ("\n***[" + debugV[el.second] + " - " + el.first + "]***");
        for (const auto &el : Code)
            f << el << "\n";
        f.close();
    };
    //****************************************************************
    // выделение операндов для директив
    auto GetParams = [](int CountOfParams,int &pos, const std::vector<Name> &L, std::function<bool(uint8_t)> F)
    {
        std::vector<std::string> res, error(1, "-1");

        pos++;
        // если директива находится в конце файла и не имеет операндов
        // возвращается ошибка
        if (pos >= L.size())
            return error;
        // считываются необходимые параметры после директивы
        while (F(L[pos].second) && pos < L.size() )
        {
            res.push_back(L[pos].first);
            pos++;
        }
        // если после директивы не оказалось операндов или их было недостаточно
        // возвращается ошибка
        if (CountOfParams != -1)
            if ( res.size() != CountOfParams)
                return error;
        pos--;
        return res;
    };
    //****************************************************************
    // выполнение директив, принимает: кол-во операндов для директивы, поток, куда буду записываться
    // команды процессора, условие для операндов -> F, то есть опред. типы операндов
    auto DoCommand = [&](int CountOfParams, int &pos, BinaryFstream &bf,
                std::function<bool(uint8_t)> F) -> void{
        int cod = Lexems[pos].second; // код выполняемой директивы
        auto Op = GetParams(CountOfParams, pos, Lexems, F); // операнды для директивы
        //std::cout << "Операнды взяты --> |";
        //for (auto el : Op) std::cout << el << " ";
        // проверка на ошибки (количество операндов)
        if (Op[0] == "-1")
            throw Assembler::Message("Неверные операнды или их недостаточно");
        //std::cout << "|выполнение диррективы " << debugV[cod] << "\n";
        // выполнение директивы
        (*(D[cod]))(*this, Op, bf);
    };
    //****************************************************************
    // проверка на ошибки
    for (auto el : Lexems) {
        if (el.second >= Assembler::ErrorWrongEndTag) {
            Listing(FileName);
            return false;
        }
    }
    //****************************************************************
    BinaryFstream bf(FileName);
    int i = 0, numbOfDir = 0;
    try {
        while (i < Lexems.size()) {
            numbOfDir = i;
            switch (Lexems[i].second) {
                case Assembler::dMov:
                    DoCommand(2, i, bf, [](uint8_t p) { return (p == Assembler::Operand); });
                    break;
                case Assembler::dUpld:
                    DoCommand(2, i, bf, [](uint8_t p) { return (p == Assembler::Operand || p == Assembler::Number); });
                    break;
                case Assembler::dIn:
                    DoCommand(2, i, bf,
                                [](uint8_t p) { return (p == Assembler::Operand || p == Assembler::TagInfo);
                    });
                    break;
                case Assembler::dOut:
                    DoCommand(2, i, bf,
                                [](uint8_t p) { return (p == Assembler::Operand || p == Assembler::TagInfo);
                    });
                    break;
                case Assembler::dCall:
                    DoCommand(1, i, bf,
                                [](uint8_t p) { return (p == Assembler::Operand);
                                });
                    break;
                case Assembler::dTag:
                    if (DefineType(Lexems[i+1].first) == Assembler::tTAG) { // метка явл-я функцией
                        TagTable.at(Lexems[i].first) = LFMA.LastCommand;
                        DoCommand(1, i, bf, [](uint8_t p) { return (p == Assembler::TagInfo); });
                    } else { // метка определяет некоторые данные
                        TagTable.at(Lexems[i].first) = LFMA.LastData;
                        DoCommand(-1, i, bf, [](uint8_t p) {
                            return (p == Assembler::TagInfo || p == Assembler::Operand ||
                                    p == Assembler::Number  || p == Assembler::Character);
                        });
                    }
                    break;
                case Assembler::EndTag: {
                    std::vector<std::string> Op(1, Lexems[i].first);
                    (*(D[Assembler::dEnd]))(*this, Op, bf);
                    }
                    break;
                case Assembler::dJmp:
                case Assembler::dJmpa:
                case Assembler::dJmpae:
                case Assembler::dJmpb:
                case Assembler::dJmpbe:
                case Assembler::dJmpe:
                case Assembler::dJmpne:
                    DoCommand(-1, i, bf,
                              [](uint8_t p) { return (p == Assembler::Operand);
                              });
                    break;
                default: {
                    if (Lexems[i].second >= dAdd && Lexems[i].second <= dCmp)
                        DoCommand(3, i, bf, [](uint8_t p) {
                            return (p == Assembler::TagInfo || p == Assembler::Operand);
                        });
                    if (Lexems[i].second >= dAddR && Lexems[i].second <= dInit)
                        DoCommand(3, i, bf, [](uint8_t p) {
                            return (p == Assembler::TagInfo || p == Assembler::Operand || p == Assembler::Number);
                        });
                }
                        break;
            }
            i++;
            //std::cout << " LastCOP " << LFMA.LastCommand << " LastData " << LFMA.LastData << "\n";
        }
        bf << std::string("IP") << TagTable["MAIN"]; // указываем начало работы программы
    } catch (Message &message) {
        Code[Lexems[numbOfDir].third] += ("\n***[" + message  + "]***");
        Listing(FileName);
        std::cerr << "******<[" <<message << "]>******" << "\n";
        bf.clear();
        return false;
    };
    bf.close();
    return true;
}

bool Assembler::Run(const std::string &FileName, const std::string &outputBIN) noexcept {
    auto begin = std::chrono::steady_clock::now(); // таймер
    if (!FirstBypassing(FileName)) {
        std::cout << "Не найдена точка входа MAIN! Завершение работы!\n";
        return false;
    }
    if (!SecondBypassing(outputBIN)){
        std::cout << "Обнаружены ошибки! листинг: " << (FileName+"_listing") << "\n";
        return false;
    }
    auto end = std::chrono::steady_clock::now();
    std::cout << "Время работы: " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() <<'\n';
    std::cout << "Бинарный файл был успешно создан!\n";
    return true;
}

std::ostream &operator<<(std::ostream &os, const Assembler &a) {
    os << "Lexems:\n";
    for (auto el : a.Lexems)
        os << el.first << " " << a.debugV[el.second] << " " << el.third << "\n";
    os << "************************************\n";
    os << "TagTable:\n";
    for (auto el : a.TagTable)
        os << el.first << " " << el.second << "\n";
    os << "************************************\n";
    os << "RegsTable:\n";
    for (auto el : a.RegsTable)
        os << el.first << " " << el.second << "\n";
    return os;
}


void Assembler::AddReg(const Assembler::TableEl &el) {
    RegsTable.insert(el);
}


