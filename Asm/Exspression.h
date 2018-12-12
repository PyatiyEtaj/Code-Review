#ifndef ASSEMBLER_EXSPRESSION_H
#define ASSEMBLER_EXSPRESSION_H

#include <iostream>
#include <map>


// контекст
template <class T>
class Context{
private:
    // таблица имен
    std::map< std::string, T > data;
public:
    using Value = std::pair<std::string, T >;
    void Add(const Value &v){
        data.insert(v);
    }
    uint16_t operator [](std::string &key){
        return data[key];
    }
};

// абстрактное выражение, которое, в зависимости от сутиации
// интерперетирует контекст
template <class T> // весь класс обернул в шаблон т.к.
// только виртуальный метод нельзя обернуть
class Expression{
public:
    virtual T Interpret(Context<T> context) = 0;
};

// терминальное выражение для переменных
template <class T>
class VarExpression : public Expression<T>{
private:
    std::string var;
    bool constVar = false;
public:
    explicit VarExpression(const std::string &name);
    T Interpret(Context<T> context) override;
};

// не терминальное выражение
template <class T>
class SignExpression : public Expression<T>{
private:
    Expression<T> *left;
    Expression<T> *right;
    int8_t sign;
    enum AllSigns : int8_t {Sum, Sub, Mul, Div, Err};
    int8_t Convert(const std::string &str);
public:
    SignExpression(Expression<T>* l, Expression<T>* r, const std::string &sign){
        left = l;
        right = r;
        this->sign = Convert(sign);
    }
    T Interpret(Context<T> context) override;
};

template <class T>
VarExpression<T>::VarExpression(const std::string &name) {
    constVar = true;
    std::string buf;
    for (auto el : name)
        if ((el >= 'a' && el <= 'z') || (el >= 'A' && el <= 'Z')) {
            constVar = false;
            buf += el;
        }
    // если число имеет вид 1.0e-12, то считаем его за костанту
    // которую в последствие можно будет интерпретировать
    if (buf.size() == 1)
        if (buf[0] == 'e')
            constVar = true;
    var = name;
}
// спезиализация шаблоная для разных типов
template <class T>
T VarExpression<T>::Interpret(Context<T> context) {
    if (constVar) return static_cast<T>(std::stoi(var));
    return context[var];
}

// inline - необходим т.к. специализация не зависит от параметра шаблона
// если inline отсутствует -> будет нарушено правило определения

template <>
inline float VarExpression<float>::Interpret(Context<float> context) {
    if (constVar) return std::stof(var);
    return context[var];
}

template <>
inline uint VarExpression<uint>::Interpret(Context<uint> context) {
    if (constVar) return static_cast<uint>(std::stoul(var));
    return context[var];
}

template <class T>
T SignExpression<T>::Interpret(Context<T> context) {
    switch (sign)
    {
        case AllSigns::Sum:
            return left->Interpret(context) + right->Interpret(context);
        case AllSigns::Sub:
            return left->Interpret(context) - right->Interpret(context);
        case AllSigns::Mul:
            return left->Interpret(context) * right->Interpret(context);
        case AllSigns::Div:
            return left->Interpret(context) / right->Interpret(context);
        default:
            throw std::string("Неверный знак!");
    }
}

template <class T>
int8_t SignExpression<T>::Convert(const std::string &str) {
    if (str == "+")
        return AllSigns::Sum;
    if (str == "-")
        return AllSigns::Sub;
    if (str == "*")
        return AllSigns::Mul;
    if (str == "/")
        return AllSigns::Div;
    return AllSigns::Err;
}



#endif //ASSEMBLER_EXSPRESSION_H
