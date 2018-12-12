cmake_minimum_required(VERSION 3.5)
project(Assembler)

set(CMAKE_CXX_STANDARD 14)

add_executable(Assembler main.cpp AdditionalFunctions.h CodeOfCommands.h BinaryFstream.h
                BinaryFstream.cpp structs.h Assembler.cpp Assembler.h AsmCommands.cpp AsmCommands.h
                Lexer.h Lexer.cpp Interpreter.h Interpreter.cpp Exspression.h)