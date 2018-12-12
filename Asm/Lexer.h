#ifndef ASSEMBLER_LEXER_H
#define ASSEMBLER_LEXER_H

#include <vector>
#include <algorithm>
#include <iostream>
#include <fstream>

#define DEBUG true

class Lexer
{
public:
    struct Lexem{
        std::string first = ""; // сама лексема
        uint8_t second = 0; // тип лексемы
        uint16_t third = 0; // номер строки в которой она находится
        Lexem() = default;
        Lexem(const std::string f,const uint8_t s, const uint16_t &t):first(f),second(s),third(t){};
    };
private:
    std::string text;
    std::vector<std::string> KeyWords, Code;
    int length, pos = 0;
    uint16_t  LatestLine = 0;
    std::vector<Lexem> lexems;
public:
    const static unsigned char EndLine = 3;
    enum symbols {COMMENT, KEY_WORDS, LPAR, RPAR, LBRCT, RBRCT, COLON, CHARACTER, IDENT, COMMA, END_LINE, END_TAG,
                  ERROR, SIGN, END };
#if DEBUG
    std::vector<std::string> debugV = {"COMMENT", "KEY_WORDS", "LPAR", "RPAR", "LBRCT", "RBRCT", "COLON", "CHARACTER", "IDENT", "COMMA",
                                       "END_LINE", "END_TAG", "ERROR", "SIGN","END" };
#endif
    Lexer()
    {
        std::ifstream f("KeyWords");
        if (!f.is_open()){
            return;
        }
        while (!f.eof())
        {
            std::string tmp;
            std::getline(f, tmp);
            KeyWords.push_back(tmp);
        }
    }
    bool ReadFile(const std::string &FileName) noexcept; // чтение файла и последующий перевод в лексемы
    bool ReadString(const std::string &Input) noexcept; // чтение входной строки и перевод в лексемы
    Lexem NextTok() noexcept;
    bool eot() const noexcept {return (pos >= length);}
    std::vector<Lexem> GetAllLexems() const noexcept { return lexems; }
    std::vector<std::string> GetCode() const noexcept { return Code; }
};


#endif //ASSEMBLER_LEXER_H
