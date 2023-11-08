#ifndef SYNTAXANALYZER_H
#define SYNTAXANALYZER_H

#include <map>
#include <fstream>
#include "WordAnalyzer.h"
#include "Error.h"
#include "SymbolTable.h"
#include "MiddleCode.h"

using namespace std;

class SyntaxAnalyzer {
private:
    WordAnalyzer wordAnalyzer;
    Error error;
    SymbolTable symbolTable;
    MiddleCode middleCode;
    Word word[3];
    Word lastword;
    Symbol nowsymbol;
    int tmpIndex;
    int tmpLableId;
    baseType nowFuncRet;
    int whilenum;
    int ifId;
    int whileId;
    stack<int> whileIdStack;
    map<std::string, std::string> idMap;
    vector<string> syntaxcom;
    ofstream outputfile;
    void output(const std::string& syntaxContent);
    void output(const Word& outputword);
    void getWord();
    void addToMap(const string& id, const string& type);
    static bool isUnaryExp(const Word& w);
    static bool isStmt(const Word& w);

    static int expNumInString(const string& content);
    string checkString();

    void CompUnit();
    void Decl();
    void ConstDecl();
    baseType BType();
    void ConstDef(baseType bt);
    vector<int> ConstInitVal();
    void VarDecl();
    void VarDef(baseType bt);
    vector<struct Symbol> InitVal();
    void FuncDef();
    void MainFuncDef();
    baseType FuncType();
    void FuncFParams();
    void FuncFParam();
    void Block();
    void BlockItem();
    void Stmt();
    Symvalue Exp(int mode);
    void Cond(int nowid, const string& type);
    Symvalue LVal(int mode);
    Symvalue PrimaryExp(int mode);
    int Number();
    int Char();
    Symvalue UnaryExp(int mode);
    void UnaryOp();
    vector<struct Symvalue> FuncRParams(const Word& word1);
    Symvalue MulExp(int mode);
    Symvalue AddExp(int mode);
    void RelExp();
    void EqExp();
    void LAndExp(int tmpLabel);
    void LOrExp(int nowid, const string& type);
    int ConstExp();
    bool checkEq();
    void checkLval();
    void checkExp();
    void checkMulExp();
    void checkUnaryExp();
    void checkPrimaryExp();
    void checkFuncRParams();
    string newTmpName();

public:
    SyntaxAnalyzer(WordAnalyzer &newWordAnalyzer, Error &newError, SymbolTable &newSymbolTable, MiddleCode &newMiddleCode);
    void analyze();

};

#endif //SYNTAXANALYZER_H
