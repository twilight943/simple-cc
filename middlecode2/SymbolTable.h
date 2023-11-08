#include <vector>
#include <string>

#ifndef WITHERROR_SYMBOLTABLE_H
#define WITHERROR_SYMBOLTABLE_H

using namespace std;

enum baseType {INT,VOID,CHAR,AUTO,NOBASETYPE};

enum dataType {CONST,VAR,FUNC,PARAM,NODATATYPE,NOFUNC};

struct Symvalue {
    baseType basetype = NOBASETYPE;
    int dim = 0;
    int value = 0;
};

struct Symbol {
    Symbol(string n, dataType dt, baseType bt, int dim, int dim2);
    Symbol(string n, dataType dt, baseType bt, int dim, bool pointer);
    Symbol(string n, dataType dt, baseType bt, int dim);
    string name;
    baseType basetype;
    dataType datatype;
    vector<Symbol> params;
    int dim;
    int dim1;
    int dim2;
    vector<int> constvalue;
    int value;
    int offset;
    bool pointer;
    Symbol(string n, dataType dt, baseType bt);
    Symbol(string n, int v);
    Symbol();
};

class SymbolTable {
private:
    vector<Symbol> globalTable;
    vector<vector<Symbol>> localTables;
    vector<Symbol> nowlocalTable;
    bool global;

public:
    friend class MiddleCode;
    SymbolTable();
    bool insert(const string& name, dataType dt, baseType bt);
    bool findInNowFunc(const string &n, dataType dt);
    dataType findDTInAllFunc(const string& n, dataType dt);
    dataType findDTInAllFunc(const string& n);
    baseType findBTInAllFunc(const string& n, dataType dt);
    Symbol findSymbol(const string& n, dataType dt);
    void clearNowFunc();
    void addNewLocalTable();
    void removeNowLocalTable();
    std::vector<Symbol> getFuncParams(const string& n);
    void changeDimType(int dim, int dim1, int dim2);
    void changeConstValue(const vector<int>& value);
    void changeValue(int value);
    void changeBasetype(baseType bt);
    baseType getNowBaseType();
    Symbol getNowSymbol();
    Symbol getNowFunc();
    static bool checkDt(dataType dt, const Symbol& it);
    int getConstValue(const string &n);
    bool insert(const string &n, dataType dt, baseType bt, int dim);
    bool insert(const string &n, dataType dt, baseType bt, int dim, int dim2);
    pair<bool, int> getOffset(const string& name);
};


#endif //WITHERROR_SYMBOLTABLE_H
