cmake_minimum_required(VERSION 3.20)
project(middlecode2)

set(CMAKE_CXX_STANDARD 11)

add_executable(middlecode2 main.cpp SyntaxAnalyzer.cpp SyntaxAnalyzer.h WordAnalyzer.cpp WordAnalyzer.h Error.cpp Error.h SymbolTable.cpp SymbolTable.h MiddleCode.cpp MiddleCode.h)
