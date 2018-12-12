cmake_minimum_required(VERSION 3.5)
project(VM)

set(CMAKE_CXX_STANDARD 14)

add_executable(VM main.cpp processor.h structs.h processor.cpp Command.h Command.cpp ArithemiticFunc.h BinaryFstream.h BinaryFstream.cpp AdditionalFunctions.h CodeOfCommands.h Debugger.h Debugger.cpp)