#include "Interpreter.h"

bool Interpreter::reversePolishNotation() noexcept {
    // приоритет операций
    auto priority = [](std::string sign) -> int {
        if (sign == "+" || sign == "-")
            return 0;
        if (sign == "*" || sign == "/")
            return 1;
        return -1;
    };
    std::stack<std::string> s;
    int i = 0;
    for (auto el : lexems){
        //std::cout << el.first << " " << el.second << "\n";
        switch (el.second)
        {
            // различные значения выталкиваем в готовую строку
            case Lexer::IDENT:
                rpnOutput.emplace_back(el.first, Lexer::IDENT);
                break;
             // скобки заталкиваем в стек
            case Lexer::LBRCT:
                s.push(el.first);
                break;
            // выталкиваем в готовую строку все из стека, пока не найдем (
            // если не нашли -> неверная строка
            case Lexer::RBRCT:{
                // если в стеке нет данных и текущая
                // лексема ')' -> неверное кол-во скобок
                if (s.empty())
                    return false;
                auto sign = s.top();
                while (sign != "(" && !s.empty()){
                    rpnOutput.emplace_back(sign, Lexer::SIGN);
                    s.pop();
                    sign = s.top();
                }
                if (sign != "(") return false;
                if (!s.empty()) s.pop();
            }
                break;
            // все знаки, имеющий более высокий приоритет и  находящиеся в стеке
            // выталкиваем в готовую строку (rpnOutput)
            // а входной знак заталкиваем в стек
            case Lexer::SIGN:{
                auto p = priority(el.first);
                auto tmp = el.first;
                if (!s.empty()){
                    std::string sign = s.top();
                    while (p <= priority(sign) && !s.empty()){
                        rpnOutput.emplace_back(sign, Lexer::SIGN);
                        s.pop();
                        if (!s.empty()) sign = s.top();
                    }
                }
                s.push(tmp);
            }
                break;
            default:
                break;
        }
        i++;
    }
    // пока стек не пуст выталкиваем все из него в готовую строку (rpnOutput)
    while (!s.empty())
    {
        rpnOutput.emplace_back(s.top(), Lexer::SIGN);
        s.pop();
    }
    return true;
}

