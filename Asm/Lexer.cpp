#include "Lexer.h"

bool Lexer::ReadFile(const std::string &FileName) noexcept
{
    std::ifstream f(FileName);
    std::string tmp;
    if (!f.is_open())
        return false;
    while (!f.eof())
    {
        tmp = "";
        std::getline(f, tmp);
        Code.push_back(tmp);
        tmp += EndLine;
        text += tmp;
    }
    length = int(text.length());
    f.close();
    Lexem l;
    while (l.second != Lexer::END)
    {
        l = NextTok();
        lexems.push_back(l);
    }
    return true;
}

bool Lexer::ReadString(const std::string &Input) noexcept
{
    text = Input;
    length = int(text.length());
    Lexem l;
    while (l.second != Lexer::END)
    {
        l = NextTok();
        lexems.push_back(l);
    }
    return true;
}

Lexer::Lexem Lexer::NextTok() noexcept
{
    std::string symbol; int sym = 0;
    auto IsLitera = [](char ch) -> bool // проверка символа, является ли он каким-либо символом и не цифрой
    {
        return (ch == '_' || (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z'));
    };
    auto IsDigit = [](char ch) -> bool  // проверка символа, является ли он цифрой
    {
        return (ch >= '0' && ch <= '9');
    };
    // выделение идентификаторов пример: ident123
    auto identificator = [IsLitera, IsDigit](const std::string &txt, int &pos)
    {
        std::string ident;
        while ( IsLitera(txt[pos]) || IsDigit(txt[pos]))
        {
            ident += txt[pos];
            pos++;
        }
        return ident;
    };
    // выделение чисел, примеры: 123, 1е-123, 0x1234 и 0.123
    auto number = [IsDigit, IsLitera](const std::string &txt, int &pos)
    {
        std::string ident; // результат
        bool exponent_ = false, hex_ = false;
        while ( IsDigit(txt[pos]) || (exponent_ && (txt[pos] == '-' || txt[pos] == '+'))
                || (hex_ && IsLitera(txt[pos])) || txt[pos] == '.' )
        {
            ident += txt[pos];
            pos++;
            if (txt[pos] == 'e')
            {
                exponent_ = true;
                ident += 'e'; pos++;
            }
            if (txt[pos] == 'x')
            {
                hex_ = true;
                ident += 'x'; pos++;
            }
        }
        if (IsLitera(txt[pos])){ // ошибка после цифр идут символы
            while (IsDigit(txt[pos]) || IsLitera(txt[pos]))
            { ident += txt[pos]; pos++; }
            ident += "*";
        }
        return ident;
    };
    // все возможные пробелы, концы строк пропускаются
    while ((text[pos] == ' ' || text[pos] == '\n') && (pos < length))
        pos++;
    // проверка на выход за границу
    if (pos >= length)
    {
        sym = END;
        return Lexem("",uint8_t (sym), LatestLine);
    }
    switch (text[pos])
    {
        case '[': sym = LPAR; symbol = "["; break;
        case ']': sym = RPAR; symbol = "]"; break;
        case '(': sym = LBRCT; symbol = "("; break;
        case ')': sym = RBRCT; symbol = ")"; break;
        case ',': sym = COMMA; symbol = ","; break;
        case ':': sym = COLON; symbol = ":"; break;
        case '\'': // выделение подобных строк 'asd' или "asd"
        case '\"': {
            pos++;
            sym = CHARACTER;
            while (text[pos] != '\'' && text[pos] != '\"') {
                if (text[pos] == '\\' && text[pos+1] == 'n')
                {
                    symbol += '\n';
                    pos++;
                } else
                {
                    symbol += text[pos];
                }
                pos++;
            }
            break;
        }
        case EndLine: sym = END_LINE; symbol = "EndLine"; LatestLine++;break;
        default : {
            if (IsLitera(text[pos])) { // если обнаружена буква, то выделяем идентификатор
                sym = IDENT;
                symbol = identificator(text, pos);
                pos--;
            } else if (IsDigit(text[pos])) {
                // тоже самое, только здесь проверяется, не идут ли буквы после цифр
                // Пример: 123 - нормально, 123F - ошибка, а вот 0x123F уже нет
                sym = IDENT;
                symbol = number(text, pos);
                if (*(symbol.end()-1) == '*') {
                    sym = ERROR;
                    symbol.pop_back();
                }
                pos--;
            } else if (text[pos] == ';') // комментарии
            {
                while (text[pos] != EndLine){symbol+=text[pos]; pos++;}
                sym = COMMENT; LatestLine++;
            }
            // выделение различных знаков
            if (text[pos] == '+' || text[pos] == '-' || text[pos] == '*' || text[pos] == '/') {
                symbol = text[pos];
                sym = SIGN;
            }
            // проверка, не является ли идентификатор ключевым словом (KeyWords.txt) или тагом, указывающим на конец
            if (sym == IDENT) {
                auto kw = find_if(KeyWords.begin(), KeyWords.end(),
                                  [&symbol](std::string str) { return (str == symbol); });
                if (kw != KeyWords.end())
                    sym = KEY_WORDS;
                if (symbol == "END")
                    sym = END_TAG;
            }
            break;
        }
    }
    pos++;
    return Lexem(symbol, sym, LatestLine);
}