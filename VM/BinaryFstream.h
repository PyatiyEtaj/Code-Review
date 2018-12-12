#ifndef VM_BINARYFSTREAM_H
#define VM_BINARYFSTREAM_H

#include <iostream>
#include <fstream>

// Простой интерфейс для работы с бинарными файлами
class BinaryFstream{
private:
    std::fstream f;
    std::ios_base::openmode Flag;
    bool EndOfFile = false;
public:
    BinaryFstream() = delete;
    explicit BinaryFstream(const std::string &FileName,
                            std::ios_base::openmode Flag_ = std::ios::binary | std::ios::out) noexcept;
    void close()noexcept;
    void open(const std::string &FileName,
                std::ios_base::openmode Flag_ = std::ios::binary | std::ios::out) noexcept;
    bool eof() const noexcept {return EndOfFile;}
    bool is_open() const noexcept {return f.is_open();}
    ~BinaryFstream(){ close(); }

    template <typename T>
    friend BinaryFstream& operator << (BinaryFstream &BF, T value) {
        BF.f.write((char *) &value, sizeof(value));
        return BF;
    }
    template <typename T>
    friend BinaryFstream& operator >> (BinaryFstream &BF, T &value){
        BF.f.read((char*)&value, sizeof(value));
        if (BF.f.eof())
            BF.EndOfFile = true;
        return BF;
    }
    friend BinaryFstream& operator << (BinaryFstream &BF, const std::string& str);
    friend BinaryFstream& operator >> (BinaryFstream &BF, std::string& str);
};


#endif //VM_BINARYFSTREAM_H
