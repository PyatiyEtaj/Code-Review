#ifndef ASSEMBLER_INTERPRETER_H
#define ASSEMBLER_INTERPRETER_H

#include <stack>
#include "Assembler.h"
#include "Exspression.h"

// интерпретатор, имеет очень простой интерфейс
// на вход дается строка которую необходимо интерпретировать
// и объект assembler, для того, чтобы заменять метки и адресные
// регистры на задреса, которые они хранят
class Interpreter{
private:
    using element = std::pair<std::string, uint8_t >;
    std::vector<Lexer::Lexem> lexems;
    std::vector<element> rpnOutput;
    // перевод в обратную польскую запись
    bool reversePolishNotation() noexcept;
    // вычисление выражения
    template <class T>
    std::string calculate(Context<T> context) noexcept;
public:
    // инициализация данных, разбиение выражения на лексемы
    template <class T>
    std::string Run (const std::string &input, class Assembler &assembler);
};


template <class T>
std::string Interpreter::Run(const std::string &input, class Assembler &assembler) {
    // очищаю котнейнеры, на случай многократного исп.
    lexems.clear();
    rpnOutput.clear();
    // разделяю входную строку на лексемы
    Lexer lexer;
    lexer.ReadString(input);
    lexems = lexer.GetAllLexems();
    // тут заморочка с знаком -
    // т.к. изначально -100 разделится на 2 части на - и 100
    // и в итоге посчитается как ошибка, так как - это
    // бинарная операция. В данном куске кода минус прибавляется к IDENT
    // и удаляется из вектора lexems
    // иными словами -100 так и останется -100, не разделяясь на составные части
    auto findFirst = [&](int i)->bool{
        // лямбда, которая ищет первый попавшийся знак или идентификатор
        for (; i >= 0; i--){
            if (lexems[i].second == Lexer::SIGN)
                return true;
            else if (lexems[i].second == Lexer::IDENT)
                return false;
        }
        return true;
    };
    for (int i = 1; i < lexems.size(); i++){
        if (lexems[i].second == Lexer::IDENT) {
            if (lexems[i - 1].first == "-" && ( i==1 || findFirst(i-2))) {
                lexems[i].first = "-" + lexems[i].first;
                lexems.erase(lexems.begin()+(i-1));
                i--;
            }
        }
    }
    //**********************************************
    // произвожу перевод в обратную польскую нотацию
    if (!reversePolishNotation())
        throw Assembler::Message("Неверное кол-во скобок!");
    // получаю таблицу имен и таблицу регистров
    auto t = assembler.GetTagsTable();
    auto r = assembler.GetRegsTable();
    // создаю контекст
    Context<T> context;
    int br = 0, ids = 0, signs= 0;
    // помещаю в контекст имя-значение, а также параллельно проверяю
    // правильность мат. выражения
    for (auto el : rpnOutput) {
        switch (el.second) {
            // каждое имя в ОПН помещаем в контекст
            case Lexer::IDENT: {
                ids++;
                if (el.first[0] == 'r') // если первый элемент имени это 'r' -> это адр. регистр
                    context.Add(std::pair<std::string, T>(el.first, r[el.first]));
                else
                    context.Add(std::pair<std::string, T>(el.first, t[el.first]));
            }
                break;
            case Lexer::SIGN:
                signs++;
                break;
            default:
                br++;
                break;
        }
    }
    if (ids != (signs+1) || br % 2 != 0)
        throw Assembler::Message("Допущена ошибка в выражении, проблема со знаками " + input);
    // проивзодим расчет
    return calculate(context);
}

template <class T>
std::string Interpreter::calculate(Context<T> context) noexcept {
    std::stack<std::string> s;
    for (auto i : rpnOutput){
        switch (i.second){
            // каждый идентификатор заталкиваем в стек
            case Lexer::IDENT:
                s.push(std::to_string(VarExpression<T>(i.first).Interpret(context)));
                break;
            // если наден знак
            case Lexer::SIGN: {
                // выталкиваем из стека два значения
                // и идентифицируем их, так как они могут быть некоторыми метками или регистрами
                auto sign = i.first;
                auto right = VarExpression<T>(s.top());
                s.pop();
                auto left = VarExpression<T>(s.top());
                s.pop();
                // далее помещаем идентифицированные значения в выражение с нужным знаком
                // выполняем операцию (знак) и помещаем значение в стек
                // преобразование в строку нужно из-за того, что изначально
                // в стеке находятся идентификаторы, которые могут быть метками и регистрами
                s.push(std::to_string(SignExpression<T>(&left, &right, sign).Interpret(context)));
            }
                break;
            default:
                break;
        }
    }
    return s.top();
}


#endif //ASSEMBLER_INTERPRETER_H
