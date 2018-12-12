#include "BinaryFstream.h"
BinaryFstream::BinaryFstream(const std::string &FileName, std::ios_base::openmode Flag_) noexcept {
    f.open(FileName, Flag_);
    Flag = Flag_;
}

BinaryFstream &operator<<(BinaryFstream &BF, const std::string &str) {
    for (auto el : str)
        BF.f.write(&el, sizeof(el));
    char EndStr = 3;
    BF.f.write(&EndStr, sizeof(char));
    return BF;
}

BinaryFstream &operator>>(BinaryFstream &BF, std::string &str) {
    char symb = 1;
    const char EndStr = 3;
    while ((symb != EndStr) && (symb != '\0') && (symb != '\n'))
    {
        BF.f.read(&symb, sizeof(symb));
        str.push_back(symb);
        // помечаю, что файл закончился
        if (symb == '\0')
            BF.EndOfFile = true;
    }
    str.pop_back();
    return BF;
}

void BinaryFstream::close() noexcept {
    // указываю конец файла
    char end = '\0';
    if (Flag == (std::ios::binary | std::ios::out))
        f.write(&end, sizeof(end));
    f.close();
}

void BinaryFstream::open(const std::string &FileName, std::ios_base::openmode Flag_) noexcept {
    if (!is_open())
        f.open(FileName, Flag_);
}

