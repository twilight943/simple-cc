#include "SymbolTable.h"

using namespace std;

Symbol::Symbol(string n, dataType dt, baseType bt) : name(std::move(n)), basetype(bt), datatype(dt) {
    dim = 0;
    dim1 = dim2 = 1;
    offset = 0;
    pointer = false;
}
Symbol::Symbol(string n, dataType dt, baseType bt, int dim, bool pointer) : name(std::move(n)), basetype(bt), datatype(dt), dim(dim), pointer(pointer) {
    dim1 = dim2 = 1;
    offset = 0;
}

Symbol::Symbol(string n, dataType dt, baseType bt, int dim) : name(std::move(n)), basetype(bt), datatype(dt), dim(dim) {
    dim1 = dim2 = 1;
    offset = 0;
    pointer = false;
}

Symbol::Symbol(string n, dataType dt, baseType bt, int dim, int dim2) : name(std::move(n)), basetype(bt), datatype(dt), dim(dim), dim2(dim2) {
    dim1 = 1;
    offset = 0;
    pointer = false;
}

Symbol::Symbol(string n, int v) : name(std::move(n)), value(v) {
    if (name == "CONSTINT") {
        basetype = INT;
    } else if(name == "CONSTCHAR"){
        basetype = CHAR;
    } else {
        basetype = NOBASETYPE;
    }
    datatype = VAR;
    dim = 0;
    dim1 = dim2 = 1;
    offset = 0;
    pointer = false;
}

Symbol::Symbol() = default;

SymbolTable::SymbolTable() {
    global = true;
}

bool SymbolTable::insert(const string& n, dataType dt, baseType bt) {
    if (findInNowFunc(n, dt)) {
        return false;
    }
    if (global) {
        globalTable.emplace_back(n, dt, bt);
    } else {
        nowlocalTable.emplace_back(n, dt, bt);
    }
    if (dt == PARAM) {
        std::vector<Symbol>::reverse_iterator it;
        for (it = globalTable.rbegin(); it != globalTable.rend(); it++) {
            if ((*it).datatype == FUNC) {
                (*it).params.emplace_back(n, dt, bt);
                break;
            }
        }
    }
    return true;
}

bool SymbolTable::insert(const string& n, dataType dt, baseType bt, int dim) {
    if (findInNowFunc(n, dt)) {
        return false;
    }
    if (global) {
        globalTable.emplace_back(n, dt, bt, dim);
    } else {
        nowlocalTable.emplace_back(n, dt, bt, dim);
    }
    if (dt == PARAM) {
        std::vector<Symbol>::reverse_iterator it;
        for (it = globalTable.rbegin(); it != globalTable.rend(); it++) {
            if ((*it).datatype == FUNC) {
                (*it).params.emplace_back(n, dt, bt, dim);
                if (dim > 0) {
                    (*it).params.back().pointer = true;
                    globalTable.back().pointer = true;
                }
                break;
            }
        }
    }
    return true;
}

bool SymbolTable::insert(const string& n, dataType dt, baseType bt, int dim, int dim2) {
    if (findInNowFunc(n, dt)) {
        return false;
    }
    if (global) {
        globalTable.emplace_back(n, dt, bt, dim, dim2);
    } else {
        nowlocalTable.emplace_back(n, dt, bt, dim, dim2);
    }
    if (dt == PARAM) {
        std::vector<Symbol>::reverse_iterator it;
        for (it = globalTable.rbegin(); it != globalTable.rend(); it++) {
            if ((*it).datatype == FUNC) {
                (*it).params.emplace_back(n, dt, bt, dim, dim2);
                if (dim > 0) {
                    (*it).params.back().pointer = true;
                    globalTable.back().pointer = true;
                }
                break;
            }
        }
    }
    return true;
}

bool SymbolTable::checkDt(dataType dt, const Symbol& it) {
    if (dt != FUNC) {
        if (it.datatype != FUNC) {
            return true;
        }
    } else {
        if (it.datatype == FUNC) {
            return true;
        }
    }
    return false;
}

bool SymbolTable::findInNowFunc(const string& n,dataType dt) {
    if (global) {
        std::vector<Symbol>::iterator it;
        for (it = globalTable.begin(); it != globalTable.end(); it ++) {
            if ((*it).name == n) {
                if (dt == PARAM) {
                    if ((*it).datatype == PARAM) {
                        return true;
                    }
                } else {
                    if (checkDt(dt,(*it))) {
                        return true;
                    }
                }
            }
        }
    } else {
        std::vector<Symbol>::iterator it;
        for (it = nowlocalTable.begin(); it != nowlocalTable.end(); it ++) {
            if ((*it).name == n) {
                if (checkDt(dt,(*it))) {
                    return true;
                }
            }
        }
    }
    return false;
}

int SymbolTable::getConstValue(const string& n) {
    vector<vector<Symbol>>::reverse_iterator it1;
    vector<Symbol>::iterator it;
    for (it = nowlocalTable.begin(); it != nowlocalTable.end(); it++) {
        if ((*it).name == n) {
            if ((*it).datatype == CONST) {
                return (*it).constvalue.at(0);
            }
        }
    }
    for (it1 = localTables.rbegin(); it1 != localTables.rend(); ++it1) {
        for (it = (*it1).begin(); it != (*it1).end(); it++) {
            if ((*it).name == n) {
                if ((*it).datatype == CONST) {
                    return (*it).constvalue.at(0);
                }
            }
        }
    }
    for (it = globalTable.begin(); it != globalTable.end(); it++) {
        if ((*it).name == n) {
            if ((*it).datatype == CONST) {
                return (*it).constvalue.at(0);
            }
        }
    }
    return 0;
}

dataType SymbolTable::findDTInAllFunc(const string& n, dataType dt) {
    vector< vector<Symbol> >::reverse_iterator it1;
    vector<Symbol>::iterator it;
    if (dt == FUNC) {
        for (it = globalTable.begin(); it != globalTable.end(); it++) {
            if ((*it).name == n) {
                dataType itdt = (*it).datatype;
                if (itdt == FUNC) {
                    return itdt;
                }
            }
        }
    } else {
        for (it = nowlocalTable.begin(); it != nowlocalTable.end(); it++) {
            if ((*it).name == n) {
                dataType itdt = (*it).datatype;
                if (dt == CONST) {
                    if (itdt == CONST) {
                        return itdt;
                    }
                } else {
                    if (checkDt(dt, (*it))) {
                        return itdt;
                    }
                }
            }
        }
        for (it1 = localTables.rbegin(); it1 != localTables.rend(); ++it1) {
            for (it = (*it1).begin(); it != (*it1).end(); it++) {
                if ((*it).name == n) {
                    dataType itdt = (*it).datatype;
                    if (dt == CONST) {
                        if (itdt == CONST) {
                            return itdt;
                        }
                    } else {
                        if (checkDt(dt, (*it))) {
                            return itdt;
                        }
                    }
                }
            }
        }
        for (it = globalTable.begin(); it != globalTable.end(); it++) {
            if ((*it).name == n) {
                dataType itdt = (*it).datatype;
                if (dt == CONST) {
                    if (itdt == CONST) {
                        return itdt;
                    }
                } else {
                    if (checkDt(dt, (*it))) {
                        return itdt;
                    }
                }
            }
        }
    }
    return NODATATYPE;
}

dataType SymbolTable::findDTInAllFunc(const string& n) {
    vector< vector<Symbol> >::reverse_iterator it1;
    vector<Symbol>::iterator it;
    for (it = nowlocalTable.begin(); it != nowlocalTable.end(); it++) {
        if ((*it).name == n)
            return (*it).datatype;
    }
    for (it1 = localTables.rbegin(); it1 != localTables.rend(); ++it1) {
        for (it = (*it1).begin(); it != (*it1).end(); it++) {
            if ((*it).name == n)
                return (*it).datatype;
        }
    }
    for (it = globalTable.begin(); it != globalTable.end(); it++) {
        if ((*it).name == n)
            return (*it).datatype;
    }
    return NODATATYPE;
}

baseType SymbolTable::findBTInAllFunc(const string& n, dataType dt) {
    vector<vector<Symbol>>::reverse_iterator it1;
    vector<Symbol>::iterator it;
    for (it = nowlocalTable.begin(); it != nowlocalTable.end(); it++) {
        if ((*it).name == n)
            return (*it).basetype;
    }
    for (it1 = localTables.rbegin(); it1 != localTables.rend(); ++it1) {
        for (it = (*it1).begin(); it != (*it1).end(); it++) {
            if ((*it).name == n)
                return (*it).basetype;
        }
    }
    for (it = globalTable.begin(); it != globalTable.end(); it ++) {
        if ((*it).name == n)
            return (*it).basetype;
    }
    return NOBASETYPE;
}

Symbol SymbolTable::findSymbol(const string& n, dataType dt) {
    vector< vector<Symbol> >::reverse_iterator it1;
    vector<Symbol>::iterator it;
    for (it = nowlocalTable.begin(); it != nowlocalTable.end(); it++) {
        if ((*it).name == n) {
            if (dt == FUNC) {
                if ((*it).datatype == dt) {
                    return (*it);
                }
            } else if (dt == NOFUNC) {
                if ((*it).datatype != FUNC) {
                    return (*it);
                }
            }
        }
    }
    for (it1 = localTables.rbegin(); it1 != localTables.rend(); ++it1) {
        for (it = (*it1).begin(); it != (*it1).end(); it++) {
            if ((*it).name == n) {
                if (dt == FUNC) {
                    if ((*it).datatype == dt) {
                        return (*it);
                    }
                } else if (dt == NOFUNC) {
                    if ((*it).datatype != FUNC) {
                        return (*it);
                    }
                }
            }
        }
    }
    for (it = globalTable.begin(); it != globalTable.end(); it ++) {
        if ((*it).name == n) {
            if (dt == FUNC) {
                if ((*it).datatype == dt) {
                    return (*it);
                }
            } else if (dt == NOFUNC) {
                if ((*it).datatype != FUNC) {
                    return (*it);
                }
            }
        }
    }
    return {"NotFound",NODATATYPE,NOBASETYPE};
}

void SymbolTable::clearNowFunc() {
    nowlocalTable.clear();
    localTables.clear();
    global = true;
}

void SymbolTable::addNewLocalTable() {
    vector<Symbol> nlt;
    vector<Symbol>::iterator it;
    if (global) {
        if (!globalTable.empty()) {
            for (it = globalTable.end() - 1; it >= globalTable.begin();) {
                Symbol nowsym = *(it);
                if (nowsym.datatype != PARAM) {
                    break;
                } else {
                    nlt.insert(nlt.begin(),nowsym);
                    globalTable.erase(it);
                    it = globalTable.end() - 1;
                }
            }
        }
    } else {
        localTables.push_back(nowlocalTable);
    }
    global = false;
    nowlocalTable = nlt;
}

void SymbolTable::removeNowLocalTable() {
    vector<Symbol> nlt;
    if (localTables.empty()) {
        nowlocalTable = nlt;
    } else {
        nowlocalTable = localTables.back();
        localTables.pop_back();
    }
}

pair<bool, int> SymbolTable::getOffset(const string &name) {
    vector<vector<Symbol>>::reverse_iterator it1;
    vector<Symbol>::iterator it;
    for (it = nowlocalTable.begin(); it != nowlocalTable.end(); it++) {
        if ((*it).name == name) {
            return pair<bool, int>{true, (*it).offset};
        }
    }
    for (it1 = localTables.rbegin(); it1 != localTables.rend(); it1++) {
        for (it = (*it1).begin(); it != (*it1).end(); it++) {
            if ((*it).name == name) {
                return pair<bool, int>{true, (*it).offset};
            }
        }
    }
    for (it = globalTable.begin(); it != globalTable.end(); it++) {
        if ((*it).name == name) {
            return pair<bool, int>{false, (*it).offset};
        }
    }
    return pair<bool, int>{false, -1};
}

std::vector<Symbol> SymbolTable::getFuncParams(const string& n) {
    std::vector<Symbol>::iterator it;
    for (it = globalTable.begin(); it != globalTable.end(); it ++) {
        if ((*it).name == n && (*it).datatype == FUNC) {
            return (*it).params;
        }
    }
    std::vector<Symbol> emptyVector;
    return emptyVector;
}

void SymbolTable::changeDimType(int dim, int dim1, int dim2) {
    if (global) {
        (*(globalTable.end() - 1)).dim = dim;
        (*(globalTable.end() - 1)).dim1 = dim1;
        (*(globalTable.end() - 1)).dim2 = dim2;
        vector<int> init(dim1 * dim2,0);
        (*(globalTable.end() - 1)).constvalue = init;
    } else {
        (*(nowlocalTable.end() - 1)).dim = dim;
        (*(nowlocalTable.end() - 1)).dim1 = dim1;
        (*(nowlocalTable.end() - 1)).dim2 = dim2;
    }
}

void SymbolTable::changeConstValue(const vector<int>& value) {
    if (global) {
        (*(globalTable.end() - 1)).constvalue = value;
    } else {
        (*(nowlocalTable.end() - 1)).constvalue = value;
    }
}

void SymbolTable::changeBasetype(baseType bt) {
    if (global) {
        (*(globalTable.end() - 1)).basetype = bt;
    } else {
        (*(nowlocalTable.end() - 1)).basetype = bt;
    }
}

void SymbolTable::changeValue(int value) {
    if (global) {
        (*(globalTable.end() - 1)).value = value;
    } else {
        (*(nowlocalTable.end() - 1)).value = value;
    }
}

baseType SymbolTable::getNowBaseType()
{
    if (global) {
        return (*(globalTable.end()-1)).basetype;
    } else {
        return (*(nowlocalTable.end()-1)).basetype;
    }
}

Symbol SymbolTable::getNowSymbol()
{
    if (global) {
        return (*(globalTable.end()-1));
    } else {
        return (*(nowlocalTable.end()-1));
    }
}

Symbol SymbolTable::getNowFunc()
{
    vector<Symbol>::reverse_iterator it;
    for (it = globalTable.rbegin(); it != globalTable.rend(); it++) {
        if ((*it).datatype == FUNC) {
            return (*it);
        }
    }
    return (*(globalTable.end()-1));
}
