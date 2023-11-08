#ifndef MIDDLECODE_MIDDLECODE_H
#define MIDDLECODE_MIDDLECODE_H

#define GPMAX 9999
#define SPMAX 99999

#include <stack>
#include <map>
#include <fstream>
#include "SymbolTable.h"

using namespace std;

enum Operator
{
    Add,Minus,Mul,Div,Mod,
    GetInt,Printf,
    Declare,Assign,ArrayGet,ArrayGetArray,ArrayAssign,
    FuncDeclare,FuncCall,FuncReturn,PushParam,BlockBegin,BlockEnd,
    Lss,Leq,Gre,Geq,Eql,Neq,Not,And,Or,
    Bge,Bgt,Ble,Blt,Beq,Bne,
    Label,Jump
};

struct MiddleCodeItem
{
    MiddleCodeItem();
    MiddleCodeItem(Operator op, Symbol id1, Symbol id2, Symbol id3);
    Operator op;
    Symbol id1;
    Symbol id2;
    Symbol id3;
};

class MiddleCode {
private:
    Symbol nullSymbol = Symbol("null", NODATATYPE, NOBASETYPE);
    vector<MiddleCodeItem> middleCodeItems;
    vector<string> strs;
    vector<string> mipsStmts;
    SymbolTable nowSymbolTable;
    vector<SymbolTable> beSymbolTables;
    stack<Symbol> paramStack;
    string regS[8];
    string regT[6];
    bool dirty[6];
    int regSIndex = 0;

public:
    friend class SymbolTable;
    MiddleCode();
    int addStr(const std::string& str);
    void addMiddleCode(Operator op);
    void addMiddleCode(Operator op, Symbol id1);
    void addMiddleCode(Operator op, Symbol id1, Symbol id2);
    void addMiddleCode(Operator op, Symbol id1, Symbol id2, Symbol id3);
    int getRegS(const string& name);
    int getRegT(const string& name);
    int findNewRegT();
    string load(const string &name, const string& grf);
    string save(const string& name, const string& grf);
    void outputMips();
    void outputMiddleCode();
    void handleAll();
    void handleStrs();
    void handleGlobleVars();
    void handleFunc(const string &name);
    void programEnd();

};


#endif //MIDDLECODE_MIDDLECODE_H
