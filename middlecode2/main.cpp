#include <iostream>
#include "WordAnalyzer.h"
#include "SyntaxAnalyzer.h"

using namespace std;

int main() {
    ifstream input;
    input.open("testfile.txt");
    WordAnalyzer wordAnalyzer(input);
    ofstream erroroutput;
    erroroutput.open("error.txt");
    Error error(erroroutput);
    SymbolTable symbolTable;
    MiddleCode middleCode;
    SyntaxAnalyzer syntaxAnalyzer(wordAnalyzer, error, symbolTable, middleCode);
    syntaxAnalyzer.analyze();
    erroroutput.close();
    input.close();
    return 0;
}
