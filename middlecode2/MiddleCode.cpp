#include <iostream>
#include "MiddleCode.h"

using namespace std;

MiddleCodeItem::MiddleCodeItem(Operator op, Symbol id1, Symbol id2, Symbol id3) : op(op), id1(id1), id2(id2), id3(id3){}

MiddleCodeItem::MiddleCodeItem() = default;

MiddleCode::MiddleCode() {
    strs.emplace_back("\\n");
}

int MiddleCode::addStr(const string& str) {
    int index = (int) strs.size();
    strs.emplace_back(str);
    return index;
}

void MiddleCode::addMiddleCode(Operator op) {
    middleCodeItems.emplace_back(MiddleCodeItem(op,nullSymbol,nullSymbol,nullSymbol));
}

void MiddleCode::addMiddleCode(Operator op, Symbol id1) {
    middleCodeItems.emplace_back(MiddleCodeItem(op,id1,nullSymbol,nullSymbol));
}

void MiddleCode::addMiddleCode(Operator op, Symbol id1, Symbol id2) {
    middleCodeItems.emplace_back(MiddleCodeItem(op,id1,id2,nullSymbol));
}

void MiddleCode::addMiddleCode(Operator op, Symbol id1, Symbol id2, Symbol id3) {
    middleCodeItems.emplace_back(MiddleCodeItem(op,id1,id2,id3));
}

int MiddleCode::getRegS(const string& name)
{
    for (int i = 0; i < 8; i ++) {
        if (regS[i] == name) {
            return i;
        }
    }
    return -1;
}

int MiddleCode::getRegT(const string& name)
{
    for (int i = 0; i < 6; i ++) {
        if (regT[i] == name) {
            return i;
        }
    }
    return -1;
}

int MiddleCode::findNewRegT()
{
    for (int i = 0; i < 6; i ++) {
        if (dirty[i] == 0) {
            return i;
        }
    }
    return -1;
}

string MiddleCode::save(const string& name, const string& grf) {
    string stmt;
    pair<bool, int> result = nowSymbolTable.getOffset(name);
    bool notglobal = result.first;
    int offset = result.second;
    if (offset == -999999) {
        stmt = "add $s" + to_string(getRegS(name)) + ", " + grf + ", $0";
    } else {
        string pos = notglobal ? "$sp" : "$gp";
        stmt = "sw " + grf + ", " + to_string(offset) + "(" + pos + ")";
    }
    return stmt;
}

string MiddleCode::load(const string& name, const string& grf) {
    string stmt;
    pair<bool, int> result = nowSymbolTable.getOffset(name);
    bool notglobal = result.first;
    int offset = result.second;
    int Tindex = getRegT(name);
    if (offset == -999999) {
        stmt = "add " + grf + ", $s" + to_string(getRegS(name)) + ", $0";
    } else if (Tindex != -1) {
        stmt = "add " + grf + ", $t" + to_string(Tindex + 3) + ", $0";
    } else {
        string pos = notglobal ? "$sp" : "$gp";
        stmt = "lw " + grf + ", " + to_string(offset) + "(" + pos + ")";
    }
    return stmt;
}

void MiddleCode::handleAll() {
    outputMiddleCode();
    handleStrs();
    handleGlobleVars();
    vector<MiddleCodeItem>::iterator it;
    for (it = middleCodeItems.begin(); it != middleCodeItems.end(); it++) {
        if ((*it).op == FuncDeclare) {
            handleFunc((*it).id1.name);
        }
    }
    programEnd();
    outputMips();
}

void MiddleCode::handleStrs() {
    int stringindex = 0;
    vector<string>::iterator it;
    mipsStmts.emplace_back(".data");
    for (it = strs.begin(); it != strs.end(); it++) {
        string stmt = "str" + to_string(stringindex) + ": .asciiz \"" + (*it) + "\"";
        mipsStmts.emplace_back(stmt);
        stringindex++;
    }
    mipsStmts.emplace_back("\n");
}

void MiddleCode::handleGlobleVars() {
    int offset = 0;
    string stmt;
    Symbol symbol1("null", -1);
    Symbol symbol2("null", -1);
    Symbol symbol3("null", -1);
    mipsStmts.emplace_back(".text");
    vector<MiddleCodeItem>::iterator it;
    for (it = middleCodeItems.begin(); it != middleCodeItems.end(); it ++) {
        MiddleCodeItem item = *it;
        Operator op = item.op;
        if (op == FuncDeclare) {
            break;
        }
        if (op == Declare) {
            symbol1 = item.id1;
            symbol1.offset = offset;
            nowSymbolTable.globalTable.emplace_back(symbol1);
            if (symbol1.dim == 0) {
                offset += 4;
            } else {
                offset += symbol1.dim1 * symbol1.dim2 * 4;
            }
        } else if (op == ArrayAssign) {
            symbol1 = item.id1;
            symbol2 = item.id2;
            symbol3 = item.id3;
            pair<bool, int> result = nowSymbolTable.getOffset(symbol1.name);
            int base = result.second;
            string pos = "$gp";
            if (symbol2.name == "CONSTINT") {
                stmt = "li $t0, " + to_string(symbol2.value * 4);
                mipsStmts.emplace_back(stmt);
            } else {
                stmt = load(symbol2.name, "$t0");
                mipsStmts.emplace_back(stmt);
                stmt = "sll $t0, $t0, 2";
                mipsStmts.emplace_back(stmt);
            }
            stmt = "addi $t0, $t0, " + to_string(base);
            mipsStmts.emplace_back(stmt);
            stmt = "addu $t0, $t0, " + pos;
            mipsStmts.emplace_back(stmt);
            if (symbol3.name == "CONSTINT") {
                stmt = "li $t1, " + to_string(symbol3.value);
            } else {
                stmt = load(symbol3.name, "$t1");
            }
            mipsStmts.emplace_back(stmt);
            stmt = "sw $t1, 0($t0)";
            mipsStmts.emplace_back(stmt);
        } else if (op == ArrayGet) {
            symbol1 = item.id1;
            symbol2 = item.id2;
            symbol3 = item.id3;
            if (symbol2.name.find("tmp") == 0 && nowSymbolTable.getOffset(symbol2.name).second == -1) {
                symbol2.offset = offset;
                nowSymbolTable.globalTable.emplace_back(symbol2);
                offset += 4;
            }
            if (symbol3.name.find("tmp") == 0 && nowSymbolTable.getOffset(symbol3.name).second == -1) {
                symbol3.offset = offset;
                nowSymbolTable.globalTable.emplace_back(symbol3);
                offset += 4;
            }
            pair<bool, int> result = nowSymbolTable.getOffset(symbol1.name);
            int sym1Offset = result.second;
            string pos = "$gp";
            if (symbol2.name == "CONSTINT") {
                stmt = "li $t0, " + to_string(symbol2.value * 4);
                mipsStmts.emplace_back(stmt);
            } else {
                stmt = load(symbol2.name, "$t0");
                mipsStmts.emplace_back(stmt);
                stmt = "sll $t0, $t0, 2";
                mipsStmts.emplace_back(stmt);
            }
            stmt = "addi $t0, $t0, " + to_string(sym1Offset);
            mipsStmts.emplace_back(stmt);
            stmt = "addu $t0, $t0, " + pos;
            mipsStmts.emplace_back(stmt);
            stmt = "lw $t1, 0($t0)";
            mipsStmts.emplace_back(stmt);
            stmt = save(symbol3.name, "$t1");
            mipsStmts.emplace_back(stmt);
        } else if (op == Assign) {
            symbol1 = item.id1;
            symbol2 = item.id2;
            if (symbol2.name == "CONSTINT") {
                stmt = "li $t0, " + to_string(symbol2.value);
            } else {
                stmt = load(symbol2.name, "$t0");
            }
            mipsStmts.emplace_back(stmt);
            stmt = save(symbol1.name, "$t0");
            mipsStmts.emplace_back(stmt);
        } else if (op == Add || op == Minus || op == Mul || op == Div || op == Mod) {
            symbol1 = item.id1;
            symbol2 = item.id2;
            symbol3 = item.id3;
            if (symbol1.name.find("tmp") == 0 && nowSymbolTable.getOffset(symbol1.name).second == -1) {
                symbol1.offset = offset;
                nowSymbolTable.nowlocalTable.emplace_back(symbol1);
                offset += 4;
            }
            if (symbol2.name.find("tmp") == 0 && nowSymbolTable.getOffset(symbol2.name).second == -1) {
                symbol2.offset = offset;
                nowSymbolTable.nowlocalTable.emplace_back(symbol2);
                offset += 4;
            }
            if (symbol3.name.find("tmp") == 0 && nowSymbolTable.getOffset(symbol3.name).second == -1) {
                symbol3.offset = offset;
                nowSymbolTable.nowlocalTable.emplace_back(symbol3);
                offset += 4;
            }
            string reg1, reg2, reg3;
            if (symbol1.name == "CONSTINT") {
                stmt = "li $t0, " + to_string(symbol1.value);
                mipsStmts.emplace_back(stmt);
                reg1 = "$t0";
            } else if (getRegT(symbol1.name) != -1) {
                int regTindex = getRegT(symbol1.name);
                reg1 = "$t" + to_string(regTindex + 3);
                dirty[regTindex] = false;
            } else {
                stmt = load(symbol1.name, "$t0");
                mipsStmts.emplace_back(stmt);
                reg1 = "$t0";
            }
            if (symbol2.name == "CONSTINT") {
                stmt = "li $t1, " + to_string(symbol2.value);
                mipsStmts.emplace_back(stmt);
                reg2 = "$t1";
            } else if (getRegT(symbol2.name) != -1) {
                int regTindex = getRegT(symbol2.name);
                reg2 = "$t" + to_string(regTindex + 3);
                dirty[regTindex] = false;
            } else {
                stmt = load(symbol2.name, "$t1");
                mipsStmts.emplace_back(stmt);
                reg2 = "$t1";
            }
            int regTindex = findNewRegT();
            if (regTindex != -1) {
                dirty[regTindex] = true;
                regT[regTindex] = symbol3.name;
                reg3 = "$t" + to_string(regTindex+3);
            } else {
                reg3 = "$t2";
            }
            if (op == Add) {
                stmt = "addu " + reg3 + ", " + reg1 + ", " + reg2;
            } else if (op == Minus) {
                stmt = "subu " + reg3 + ", " + reg1 + ", " + reg2;
            } else if (op == Mul) {
                stmt = "mult " + reg1 + ", " + reg2;
            } else {
                stmt = "div " + reg1 + ", " + reg2;
            }
            mipsStmts.emplace_back(stmt);
            if (op == Mul || op == Div) {
                stmt = "mflo " + reg3;
                mipsStmts.emplace_back(stmt);
            } else if (op == Mod) {
                stmt = "mfhi " + reg3;
                mipsStmts.emplace_back(stmt);
            } else {}
            if (reg3 == "$t2" || symbol3.name.find("tmp") == string::npos) {
                stmt = save(symbol3.name, reg3);
                mipsStmts.emplace_back(stmt);
            }
        } else {}
    }
    mipsStmts.emplace_back("j main");
    mipsStmts.emplace_back("nop");
}

void MiddleCode::handleFunc(const string &name) {
    string stmt;
    int offset = 0;
    nowSymbolTable.clearNowFunc();
    regS->clear();
    regSIndex = 0;
    mipsStmts.emplace_back("\n");
    for (bool & i : dirty) {
        i = false;
    }
    stmt = name + ":";
    mipsStmts.emplace_back(stmt);
    vector<MiddleCodeItem>::iterator it1;
    for (it1 = middleCodeItems.begin(); it1 != middleCodeItems.end(); it1++) {
        if ((*it1).op == FuncDeclare && (*it1).id1.name == name) {
            break;
        }
    }
    auto begin = it1;
    MiddleCodeItem funcDecl = *it1;
    vector<Symbol>::iterator it2;
    for (it2 = funcDecl.id1.params.begin(); it2 != funcDecl.id1.params.end(); it2++) {
        Symbol param = *it2;
        if (regSIndex <= 7) {
            param.offset = -999999;
            regS[regSIndex] = param.name;
            regSIndex++;
        } else {
            param.offset = offset;
            offset += 4;
        }
        nowSymbolTable.nowlocalTable.emplace_back(param);
    }
    Symbol symbol1("null", -1);
    Symbol symbol2("null", -1);
    Symbol symbol3("null", -1);
    bool funcBlock = true;
    for (it1 = begin; it1 != middleCodeItems.end(); it1++) {
        MiddleCodeItem item = (*it1);
        Operator op = item.op;
        if (op == FuncDeclare && item.id1.name != name) {
            break;
        }
        if (op == BlockBegin) {
            if (funcBlock) {
                funcBlock = false;
            } else {
                nowSymbolTable.localTables.push_back(nowSymbolTable.nowlocalTable);
                vector<Symbol> nlt;
                nowSymbolTable.nowlocalTable = nlt;
            }
        } else if (op == BlockEnd) {
            nowSymbolTable.removeNowLocalTable();
        } else if (op == Declare) {
            symbol1 = item.id1;
            if (symbol1.dim == 0) {
                symbol1.offset = offset;
                offset += 4;
            } else {
                symbol1.offset = offset;
                offset += symbol1.dim1 * symbol1.dim2 * 4;
            }
            nowSymbolTable.nowlocalTable.emplace_back(symbol1);
        } else if (op == Lss || op == Leq || op == Gre || op == Geq || op == Eql || op == Neq || op == And || op == Or) {
            symbol1 = item.id1;
            symbol2 = item.id2;
            symbol3 = item.id3;
            if (symbol1.name.find("tmp") == 0 && nowSymbolTable.getOffset(symbol1.name).second == -1) {
                symbol1.offset = offset;
                nowSymbolTable.nowlocalTable.emplace_back(symbol1);
                offset += 4;
            }
            if (symbol2.name.find("tmp") == 0 && nowSymbolTable.getOffset(symbol2.name).second == -1) {
                symbol2.offset = offset;
                nowSymbolTable.nowlocalTable.emplace_back(symbol2);
                offset += 4;
            }
            if (symbol3.name.find("tmp") == 0 && nowSymbolTable.getOffset(symbol3.name).second == -1) {
                symbol3.offset = offset;
                nowSymbolTable.nowlocalTable.emplace_back(symbol3);
                offset += 4;
            }
        } else if (op == Add || op == Minus || op == Mul || op == Div || op == Mod) {
            symbol1 = item.id1;
            if (symbol1.name.find("tmp") == 0 && nowSymbolTable.getOffset(symbol1.name).second == -1) {
                symbol1.offset = offset;
                nowSymbolTable.nowlocalTable.emplace_back(symbol1);
                offset += 4;
            }
            symbol2 = item.id2;
            if (symbol2.name.find("tmp") == 0 && nowSymbolTable.getOffset(symbol2.name).second == -1) {
                symbol2.offset = offset;
                nowSymbolTable.nowlocalTable.emplace_back(symbol2);
                offset += 4;
            }
            symbol3 = item.id3;
            if (symbol3.name.find("tmp") == 0 && nowSymbolTable.getOffset(symbol3.name).second == -1) {
                symbol3.offset = offset;
                nowSymbolTable.nowlocalTable.emplace_back(symbol3);
                offset += 4;
            }
        } else if (op == GetInt) {
            symbol1 = item.id1;
            if (symbol1.name.find("tmp") == 0 && nowSymbolTable.getOffset(symbol1.name).second == -1) {
                symbol1.offset = offset;
                nowSymbolTable.nowlocalTable.emplace_back(symbol1);
                offset += 4;
            }
        } else if (op == Not) {
            symbol1 = item.id2;
            if (symbol1.name.find("tmp") == 0 && nowSymbolTable.getOffset(symbol1.name).second == -1) {
                symbol1.offset = offset;
                nowSymbolTable.nowlocalTable.emplace_back(symbol1);
                offset += 4;
            }
        } else if (op == ArrayGet) {
            symbol2 = item.id2;
            if (symbol2.name.find("tmp") == 0 && nowSymbolTable.getOffset(symbol2.name).second == -1) {
                symbol2.offset = offset;
                nowSymbolTable.nowlocalTable.emplace_back(symbol2);
                offset += 4;
            }
            symbol3 = item.id3;
            if (symbol3.name.find("tmp") == 0 && nowSymbolTable.getOffset(symbol3.name).second == -1) {
                symbol3.offset = offset;
                nowSymbolTable.nowlocalTable.emplace_back(symbol3);
                offset += 4;
            }
        } else if (op == ArrayGetArray) {
            symbol2 = item.id2;
            if (symbol2.name.find("tmp") == 0 && nowSymbolTable.getOffset(symbol2.name).second == -1) {
                symbol2.offset = offset;
                nowSymbolTable.nowlocalTable.emplace_back(symbol2);
                offset += 4;
            }
            symbol3 = item.id3;
            if (symbol3.name.find("tmp") == 0 && nowSymbolTable.getOffset(symbol3.name).second == -1) {
                symbol3.offset = offset;
                symbol3.pointer = true;
                nowSymbolTable.nowlocalTable.emplace_back(symbol3);
                offset += 4;
            }
        } else if (op == FuncCall) {
            symbol2 = item.id2;
            if (symbol2.name.find("tmp") == 0 && nowSymbolTable.getOffset(symbol2.name).second == -1) {
                symbol2.offset = offset;
                nowSymbolTable.nowlocalTable.emplace_back(symbol2);
                offset += 4;
            }
        } else {}
    }
    stmt = "addi $sp, $sp, -";
    stmt.append(to_string(offset));
    int usedoffset = offset;
    mipsStmts.emplace_back(stmt);

    nowSymbolTable.clearNowFunc();
    regS->clear();
    regSIndex = 0;
    offset = 0;
    int funcOffset = 0;
    for (it2 = funcDecl.id1.params.begin(); it2 != funcDecl.id1.params.end(); it2++) {
        Symbol param = *it2;
        if (regSIndex <= 7) {
            param.offset = -999999;
            regS[regSIndex] = param.name;
            regSIndex++;
        } else {
            param.offset = offset;
            offset += 4;
        }
        nowSymbolTable.nowlocalTable.emplace_back(param);
    }
    vector<Symbol>::iterator it3;
    int paramNumber = (int) (*begin).id1.params.size();
    int tmpParamNumber = 0;
    for (it3 = nowSymbolTable.nowlocalTable.begin(); it3 != nowSymbolTable.nowlocalTable.end(); it3++) {
        Symbol symbol = *it3;
        if (symbol.datatype == PARAM) {
            tmpParamNumber++;
            int pos = usedoffset + 12 + (paramNumber - tmpParamNumber) * 4;
            stmt = "lw $t0, " + to_string(pos) + "($sp)";
            mipsStmts.emplace_back(stmt);
            if (getRegS(symbol.name) != -1) {
                stmt = "addu $s" + to_string(getRegS(symbol.name)) + ", $t0, $0            # " + symbol.name;
            } else {
                stmt = "sw $t0, " + to_string(symbol.offset) + "($sp)        # " + symbol.name;
            }
            mipsStmts.emplace_back(stmt);
        }
    }
    funcBlock = true;
    for (it1 = begin; it1 != middleCodeItems.end(); it1++) {
        MiddleCodeItem item = (*it1);
        Operator op = item.op;
        if (op == FuncDeclare && item.id1.name != name) {
            break;
        }
        if (op == BlockBegin) {
            if (funcBlock) {
                funcBlock = false;
            } else {
                nowSymbolTable.localTables.push_back(nowSymbolTable.nowlocalTable);
                vector<Symbol> nlt;
                nowSymbolTable.nowlocalTable = nlt;
            }
        } else if (op == BlockEnd) {
            for (int i = 0; i < 6; i++) {
                if (dirty[i]) {
                    vector<Symbol>::iterator it4;
                    for (it4 = nowSymbolTable.nowlocalTable.begin(); it4 != nowSymbolTable.nowlocalTable.end(); it4++) {
                        if ((*it4).name == regT[i]) {
                            dirty[i] = false;
                            regT[i].clear();
                        }
                    }
                }
            }
            nowSymbolTable.removeNowLocalTable();
        } else if (op == Declare) {
            symbol1 = item.id1;
            if (symbol1.dim == 0) {
                symbol1.offset = offset;
                offset += 4;
            } else {
                symbol1.offset = offset;
                offset += symbol1.dim1*symbol1.dim2*4;
            }
            nowSymbolTable.nowlocalTable.emplace_back(symbol1);
        } else if (op == GetInt) {
            symbol1 = item.id1;
            if (symbol1.name.find("tmp") == 0 && nowSymbolTable.getOffset(symbol1.name).second == -1) {
                symbol1.offset = offset;
                nowSymbolTable.nowlocalTable.emplace_back(symbol1);
                offset += 4;
            }
            stmt = "li $v0, 5";
            mipsStmts.emplace_back(stmt);
            stmt = "syscall        # getint " + symbol1.name;
            mipsStmts.emplace_back(stmt);
            stmt = save(symbol1.name,"$v0");
            mipsStmts.emplace_back(stmt);
        } else if (op == Printf) {
            symbol1 = item.id1;
            symbol2 = item.id2;
            if (symbol1.name == "expr") {
                if (symbol2.name == "CONSTINT") {
                    stmt = "li $a0, " + to_string(symbol2.value);
                } else {
                    stmt = load(symbol2.name,"$a0");
                }
                mipsStmts.emplace_back(stmt);
                stmt = "li $v0, 1";
                mipsStmts.emplace_back(stmt);
                stmt = "syscall        # printf " + symbol2.name;
                mipsStmts.emplace_back(stmt);
            } else if (symbol1.name == "str") {
                stmt = "li $v0, 4";
                mipsStmts.emplace_back(stmt);
                stmt = "la $a0, " + symbol2.name + to_string(symbol2.value);
                mipsStmts.emplace_back(stmt);
                stmt = "syscall        # printf string";
                mipsStmts.emplace_back(stmt);
            } else {}
        } else if (op == Lss || op == Leq || op == Gre || op == Geq || op == Eql || op == Neq || op == And || op == Or) {
            symbol1 = item.id1;
            symbol2 = item.id2;
            symbol3 = item.id3;
            if (symbol1.name.find("tmp") == 0 && nowSymbolTable.getOffset(symbol1.name).second == -1) {
                symbol1.offset = offset;
                nowSymbolTable.nowlocalTable.emplace_back(symbol1);
                offset += 4;
            }
            if (symbol2.name.find("tmp") == 0 && nowSymbolTable.getOffset(symbol2.name).second == -1) {
                symbol2.offset = offset;
                nowSymbolTable.nowlocalTable.emplace_back(symbol2);
                offset += 4;
            }
            if (symbol3.name.find("tmp") == 0 && nowSymbolTable.getOffset(symbol3.name).second == -1) {
                symbol3.offset = offset;
                nowSymbolTable.nowlocalTable.emplace_back(symbol3);
                offset += 4;
            }
            string reg1, reg2, reg3;
            if (symbol1.name == "CONSTINT") {
                stmt = "li $t0, " + to_string(symbol1.value);
                mipsStmts.emplace_back(stmt);
                reg1 = "$t0";
            } else if (nowSymbolTable.getOffset(symbol1.name).second == -999999) {
                reg1 = "$s" + to_string(getRegS(symbol1.name));
            } else if (getRegT(symbol1.name) != -1) {
                int regTindex = getRegT(symbol1.name);
                reg1 = "$t" + to_string(regTindex + 3);
                dirty[regTindex] = false;
            } else {
                stmt = load(symbol1.name, "$t0");
                mipsStmts.emplace_back(stmt);
                reg1 = "$t0";
            }
            if (symbol2.name == "CONSTINT") {
                stmt = "li $t1, " + to_string(symbol2.value);
                mipsStmts.emplace_back(stmt);
                reg2 = "$t1";
            } else if (nowSymbolTable.getOffset(symbol2.name).second == -999999) {
                reg2 = "$s" + to_string(getRegS(symbol2.name));
            } else if (getRegT(symbol2.name) != -1) {
                int regTindex = getRegT(symbol2.name);
                reg2 = "$t" + to_string(regTindex + 3);
                dirty[regTindex] = false;
            } else {
                stmt = load(symbol2.name, "$t1");
                mipsStmts.emplace_back(stmt);
                reg2 = "$t1";
            }
            reg3 = "$t2";
            if (op == Lss) {
                stmt = "slt " + reg3 + ", " + reg1 + ", " + reg2;
                mipsStmts.emplace_back(stmt);
            } else if (op == Leq) {
                stmt = "slt " + reg3 + ", " + reg2 + ", " + reg1;
                mipsStmts.emplace_back(stmt);
                stmt = "xori " + reg3 + ", " + reg3 + ", 1";
                mipsStmts.emplace_back(stmt);
            } else if (op == Gre) {
                stmt = "slt " + reg3 + ", " + reg2 + ", " + reg1;
                mipsStmts.emplace_back(stmt);
            } else if (op == Geq) {
                stmt = "slt " + reg3 + ", " + reg1 + ", " + reg2;
                mipsStmts.emplace_back(stmt);
                stmt = "xori " + reg3 + ", " + reg3 + ", 1";
                mipsStmts.emplace_back(stmt);
            } else if (op == Eql) {
                stmt = "xor " + reg3 + ", " + reg1 + ", " + reg2;
                mipsStmts.emplace_back(stmt);
                stmt = "sltiu " + reg3 + ", " + reg3 + ", 1";
                mipsStmts.emplace_back(stmt);
            } else if (op == Neq) {
                stmt = "xor " + reg3 + ", " + reg1 + ", " + reg2;
                mipsStmts.emplace_back(stmt);
                stmt = "sltiu " + reg3 + ", " + reg3 + ", 1";
                mipsStmts.emplace_back(stmt);
                stmt = "xori " + reg3 + ", " + reg3 + ", 1";
                mipsStmts.emplace_back(stmt);
            } else if (op == And) {
                stmt = "and " + reg3 + ", " + reg1 + ", " + reg2;
                mipsStmts.emplace_back(stmt);
            } else {
                stmt = "or " + reg3 + ", " + reg1 + ", " + reg2;
                mipsStmts.emplace_back(stmt);
            }
            stmt = save(symbol3.name, reg3);
            mipsStmts.emplace_back(stmt);
        } else if (op == Not) {
            symbol1 = item.id1;
            symbol2 = item.id2;
            if (symbol2.name.find("tmp") == 0 && nowSymbolTable.getOffset(symbol2.name).second == -1) {
                symbol2.offset = offset;
                nowSymbolTable.nowlocalTable.emplace_back(symbol2);
                offset += 4;
            }
            string reg1, reg2;
            if (symbol1.name == "CONSTINT") {
                stmt = "li $t0, " + to_string(symbol1.value);
                mipsStmts.emplace_back(stmt);
                reg1 = "$t0";
            } else if (nowSymbolTable.getOffset(symbol1.name).second == -999999) {
                reg1 = "$s" + to_string(getRegS(symbol1.name));
            } else if (getRegT(symbol1.name) != -1) {
                int regTindex = getRegT(symbol1.name);
                reg1 = "$t" + to_string(regTindex + 3);
                dirty[regTindex] = false;
            } else {
                stmt = load(symbol1.name, "$t0");
                mipsStmts.emplace_back(stmt);
                reg1 = "$t0";
            }
            reg2 = "$t1";
            stmt = "sltiu " + reg2 + ", " + reg1 + ", 1";
            mipsStmts.emplace_back(stmt);
            stmt = save(symbol2.name, reg2);
            mipsStmts.emplace_back(stmt);
        } else if (op == Add || op == Minus || op == Mul || op == Div || op == Mod) {
            symbol1 = item.id1;
            symbol2 = item.id2;
            symbol3 = item.id3;
            if (symbol1.name.find("tmp") == 0 && nowSymbolTable.getOffset(symbol1.name).second == -1) {
                symbol1.offset = offset;
                nowSymbolTable.nowlocalTable.emplace_back(symbol1);
                offset += 4;
            }
            if (symbol2.name.find("tmp") == 0 && nowSymbolTable.getOffset(symbol2.name).second == -1) {
                symbol2.offset = offset;
                nowSymbolTable.nowlocalTable.emplace_back(symbol2);
                offset += 4;
            }
            if (symbol3.name.find("tmp") == 0 && nowSymbolTable.getOffset(symbol3.name).second == -1) {
                symbol3.offset = offset;
                nowSymbolTable.nowlocalTable.emplace_back(symbol3);
                offset += 4;
            }
            string reg1, reg2, reg3;
            if (symbol1.name == "CONSTINT") {
                stmt = "li $t0, " + to_string(symbol1.value);
                mipsStmts.emplace_back(stmt);
                reg1 = "$t0";
            } else if (nowSymbolTable.getOffset(symbol1.name).second == -999999) {
                reg1 = "$s" + to_string(getRegS(symbol1.name));
            } else if (getRegT(symbol1.name) != -1) {
                int regTindex = getRegT(symbol1.name);
                reg1 = "$t" + to_string(regTindex + 3);
                dirty[regTindex] = false;
            } else {
                stmt = load(symbol1.name, "$t0");
                mipsStmts.emplace_back(stmt);
                reg1 = "$t0";
            }
            if (symbol2.name == "CONSTINT") {
                stmt = "li $t1, " + to_string(symbol2.value);
                mipsStmts.emplace_back(stmt);
                reg2 = "$t1";
            } else if (nowSymbolTable.getOffset(symbol2.name).second == -999999) {
                reg2 = "$s" + to_string(getRegS(symbol2.name));
            } else if (getRegT(symbol2.name) != -1) {
                int regTindex = getRegT(symbol2.name);
                reg2 = "$t" + to_string(regTindex + 3);
                dirty[regTindex] = false;
            } else {
                stmt = load(symbol2.name, "$t1");
                mipsStmts.emplace_back(stmt);
                reg2 = "$t1";
            }
            if (nowSymbolTable.getOffset(symbol3.name).second == -999999) {
                reg3 = "$s" + to_string(getRegS(symbol3.name));
            } else {
                int regTindex = findNewRegT();
                if (regTindex != -1) {
                    dirty[regTindex] = true;
                    regT[regTindex] = symbol3.name;
                    reg3 = "$t" + to_string(regTindex+3);
                } else {
                    reg3 = "$t2";
                }
            }
            if (op == Add) {
                stmt = "addu " + reg3 + ", " + reg1 + ", " + reg2;
            } else if (op == Minus) {
                stmt = "subu " + reg3 + ", " + reg1 + ", " + reg2;
            } else if (op == Mul) {
                stmt = "mult " + reg1 + ", " + reg2;
            } else {
                stmt = "div " + reg1 + ", " + reg2;
            }
            mipsStmts.emplace_back(stmt);
            if (op == Mul || op == Div) {
                stmt = "mflo " + reg3;
                mipsStmts.emplace_back(stmt);
            } else if (op == Mod) {
                stmt = "mfhi " + reg3;
                mipsStmts.emplace_back(stmt);
            } else {}
            if (reg3 == "$t2" || symbol3.name.find("tmp") == string::npos) {
                stmt = save(symbol3.name, reg3);
                mipsStmts.emplace_back(stmt);
            }
        } else if (op == Assign) {
            symbol1 = item.id1;
            symbol2 = item.id2;
            string reg;
            if (symbol2.name == "CONSTINT") {
                stmt = "li $t0, " + to_string(symbol2.value);
                mipsStmts.emplace_back(stmt);
                reg = "$t0";
            } else if (nowSymbolTable.getOffset(symbol2.name).second == -999999) {
                reg = "$s" + to_string(getRegS(symbol2.name));
            } else if (getRegT(symbol2.name) != -1) {
                int regTindex = getRegT(symbol2.name);
                reg = "$t" + to_string(regTindex + 3);
                dirty[regTindex] = false;
            } else {
                stmt = load(symbol2.name,"$t0");
                mipsStmts.emplace_back(stmt);
                reg = "$t0";
            }
            stmt = save(symbol1.name, reg);
            mipsStmts.emplace_back(stmt);
        } else if (op == ArrayGet) {
            symbol1 = item.id1;
            symbol2 = item.id2;
            symbol3 = item.id3;
            if (symbol2.name.find("tmp") == 0 && nowSymbolTable.getOffset(symbol2.name).second == -1) {
                symbol2.offset = offset;
                nowSymbolTable.nowlocalTable.emplace_back(symbol2);
                offset += 4;
            }
            if (symbol3.name.find("tmp") == 0 && nowSymbolTable.getOffset(symbol3.name).second == -1) {
                symbol3.offset = offset;
                nowSymbolTable.nowlocalTable.emplace_back(symbol3);
                offset += 4;
            }
            pair<bool, int> result = nowSymbolTable.getOffset(symbol1.name);
            int sym1Offset = result.second;
            string pos = result.first ? "$sp" : "$gp";
            if (symbol2.name == "CONSTINT") {
                stmt = "li $t0, " + to_string(symbol2.value * 4);
                mipsStmts.emplace_back(stmt);
            } else {
                stmt = load(symbol2.name, "$t0");
                mipsStmts.emplace_back(stmt);
                stmt = "sll $t0, $t0, 2";
                mipsStmts.emplace_back(stmt);
            }
            if (symbol1.pointer) {
                stmt = load(symbol1.name, "$t2");
                mipsStmts.emplace_back(stmt);
                stmt = "addu $t0, $t0, $t2";
                mipsStmts.emplace_back(stmt);
            } else {
                stmt = "addi $t0, $t0, " + to_string(sym1Offset);
                mipsStmts.emplace_back(stmt);
                stmt = "addu $t0, $t0, " + pos;
                mipsStmts.emplace_back(stmt);
            }
            stmt = "lw $t1, 0($t0)";
            mipsStmts.emplace_back(stmt);
            stmt = save(symbol3.name, "$t1");
            mipsStmts.emplace_back(stmt);
        } else if (op == ArrayGetArray) {
            symbol1 = item.id1;
            symbol2 = item.id2;
            if (symbol2.name.find("tmp") == 0 && nowSymbolTable.getOffset(symbol2.name).second == -1) {
                symbol2.offset = offset;
                nowSymbolTable.nowlocalTable.emplace_back(symbol2);
                offset += 4;
            }
            symbol3 = item.id3;
            if (symbol3.name.find("tmp") == 0 && nowSymbolTable.getOffset(symbol3.name).second == -1) {
                symbol3.offset = offset;
                symbol3.pointer = true;
                nowSymbolTable.nowlocalTable.emplace_back(symbol3);
                offset += 4;
            }
            pair<bool, int> result = nowSymbolTable.getOffset(symbol1.name);
            int sym1Offset = result.second;
            string pos = result.first ? "$sp" : "$gp";
            if (symbol2.name == "CONSTINT") {
                stmt = "li $t0, " + to_string(symbol2.value * symbol1.dim2 * 4);
                mipsStmts.emplace_back(stmt);
            } else {
                stmt = load(symbol2.name, "$t0");
                mipsStmts.emplace_back(stmt);
                stmt = "li $t1, " + to_string(symbol1.dim2);
                mipsStmts.emplace_back(stmt);
                stmt = "mult $t0, $t1";
                mipsStmts.emplace_back(stmt);
                stmt = "mflo $t0";
                mipsStmts.emplace_back(stmt);
                stmt = "sll $t0, $t0, 2";
                mipsStmts.emplace_back(stmt);
            }
            if (symbol1.pointer) {
                stmt = load(symbol1.name, "$t2");
                mipsStmts.emplace_back(stmt);
                stmt = "addu $t0, $t0, $t2";
                mipsStmts.emplace_back(stmt);
            } else {
                stmt = "addi $t0, $t0, " + to_string(sym1Offset);
                mipsStmts.emplace_back(stmt);
                stmt = "addu $t0, $t0, " + pos;
                mipsStmts.emplace_back(stmt);
            }
            stmt = save(symbol3.name, "$t0");
            mipsStmts.emplace_back(stmt);
        } else if (op == ArrayAssign) {
            symbol1 = item.id1;
            symbol2 = item.id2;
            symbol3 = item.id3;
            pair<bool, int> result = nowSymbolTable.getOffset(symbol1.name);
            int sym1Offset = result.second;
            string pos = result.first ? "$sp" : "$gp";
            if (symbol2.name == "CONSTINT") {
                stmt = "li $t0, " + to_string(symbol2.value * 4);
                mipsStmts.emplace_back(stmt);
            } else {
                stmt = load(symbol2.name, "$t0");
                mipsStmts.emplace_back(stmt);
                stmt = "sll $t0, $t0, 2";
                mipsStmts.emplace_back(stmt);
            }
            if (symbol1.pointer) {
                stmt = load(symbol1.name, "$t2");
                mipsStmts.emplace_back(stmt);
                stmt = "addu $t0, $t0, $t2";
                mipsStmts.emplace_back(stmt);
            } else {
                stmt = "addi $t0, $t0, " + to_string(sym1Offset);
                mipsStmts.emplace_back(stmt);
                stmt = "addu $t0, $t0, " + pos;
                mipsStmts.emplace_back(stmt);
            }
            if (symbol3.name == "CONSTINT") {
                stmt = "li $t1, " + to_string(symbol3.value);
            } else {
                stmt = load(symbol3.name, "$t1");
            }
            mipsStmts.emplace_back(stmt);
            stmt = "sw $t1, 0($t0)";
            mipsStmts.emplace_back(stmt);
        } else if (op == Bne || op == Beq || op == Bge || op == Bgt || op == Ble || op == Blt ) {
            symbol1 = item.id1;
            symbol2 = item.id2;
            if (op == Beq) {
                if (symbol1.name == "CONSTINT") {
                    stmt = "li $t0, " + to_string(symbol1.value);
                } else {
                    stmt = load(symbol1.name, "$t0");
                }
                mipsStmts.emplace_back(stmt);
                stmt = "addu $t9, $t0, $0";
                mipsStmts.emplace_back(stmt);
                stmt = "beq $t0, $0, " + symbol2.name;
                mipsStmts.emplace_back(stmt);
            } else if (op == Bne) {
                stmt = "bne $t9, $0, " + symbol2.name;
                mipsStmts.emplace_back(stmt);
            } else {}
        } else if (op == Jump) {
            symbol1 = item.id1;
            stmt = "j " + symbol1.name;
            mipsStmts.emplace_back(stmt);
            mipsStmts.emplace_back("nop");
        } else if (op == Label) {
            symbol1 = item.id1;
            stmt = symbol1.name + ":";
            mipsStmts.emplace_back(stmt);
        } else if (op == PushParam) {
            symbol1 = item.id1;
            paramStack.push(symbol1);
        } else if (op == FuncCall) {
            symbol1 = item.id1;
            symbol2 = item.id2;
            if (symbol2.name.find("tmp") == 0 && nowSymbolTable.getOffset(symbol2.name).second == -1) {
                symbol2.offset = offset;
                nowSymbolTable.nowlocalTable.emplace_back(symbol2);
                offset += 4;
            }
            string savedRegT[6];
            for (int i = 0; i < 6; i++) {
                if (dirty[i]) {
                    stmt = save(regT[i], "$t" + to_string(i + 3));
                    mipsStmts.emplace_back(stmt);
                    savedRegT[i] = regT[i];
                    dirty[i] = false;
                }
            }
            for (int i = 0; i < regSIndex; i++) {
                funcOffset += 4;
                stmt = "sw $s" + to_string(i) + ", " + to_string(-funcOffset) + "($sp)";
                mipsStmts.emplace_back(stmt);
            }
            stack<Symbol> paramStackTmp;
            int paramNum = (int) symbol1.params.size();
            for (int i = 0 ; i < paramNum; i++) {
                Symbol param = paramStack.top();
                paramStack.pop();
                paramStackTmp.push(param);
            }
            while (!paramStackTmp.empty()) {
                Symbol param = paramStackTmp.top();
                paramStackTmp.pop();
                funcOffset += 4;
                if (param.name == "CONSTINT") {
                    stmt = "li $t0, ";
                    stmt.append(to_string(param.value));
                } else {
                    stmt = load(param.name, "$t0");
                }
                mipsStmts.emplace_back(stmt);
                stmt = "sw $t0, " + to_string(-funcOffset) + "($sp)";
                mipsStmts.emplace_back(stmt);
            }
            funcOffset += 4;
            stmt = "sw $ra, " + to_string(-funcOffset) + "($sp)";
            mipsStmts.emplace_back(stmt);
            funcOffset += 4;
            stmt = "sw $sp, " + to_string(-funcOffset) + "($sp)";
            mipsStmts.emplace_back(stmt);
            funcOffset += 4;
            stmt = "addi $sp, $sp, " + to_string(-funcOffset);
            mipsStmts.emplace_back(stmt);
            stmt = "jal " + symbol1.name;
            mipsStmts.emplace_back(stmt);
            mipsStmts.emplace_back("nop");
            stmt = "lw $ra, " + to_string(-funcOffset + 8) + "($sp)";
            mipsStmts.emplace_back(stmt);
            for (int i = 0; i < 6; i++) {
                if (!savedRegT[i].empty()) {
                    int savedoffset = nowSymbolTable.getOffset(savedRegT[i]).second;
                    stmt = "lw $t" + to_string(i + 3) + ", " + to_string(savedoffset) + "($sp)";
                    mipsStmts.emplace_back(stmt);
                    dirty[i] = true;
                }
            }
            for (int i = regSIndex - 1; i >= 0 ; i--) {
                int pos = -funcOffset + 8 + (regSIndex - i + paramNum) * 4;
                stmt = "lw $s" + to_string(i) + ", " + to_string(pos) + "($sp)";
                mipsStmts.emplace_back(stmt);
            }
            if (symbol2.name != "null") {
                stmt = save(symbol2.name,"$v0");
                mipsStmts.emplace_back(stmt);
            }
            funcOffset = 0;
        } else if (op == FuncReturn) {
            symbol1 = item.id1;
            if (name == "main") {
                stmt = "j programEnd";
                mipsStmts.emplace_back(stmt);
                mipsStmts.emplace_back("nop");
            } else {
                if (symbol1.name != "null") {
                    if (symbol1.name == "CONSTINT") {
                        stmt = "li $v0, " + to_string(symbol1.value);
                    } else {
                        stmt = load(symbol1.name, "$v0");
                    }
                    mipsStmts.emplace_back(stmt);
                }
                stmt = "addi $sp, $sp, " + to_string(usedoffset);
                mipsStmts.emplace_back(stmt);
                stmt = "lw $sp, 4($sp)";
                mipsStmts.emplace_back(stmt);
                stmt = "jr $ra";
                mipsStmts.emplace_back(stmt);
                mipsStmts.emplace_back("nop");
            }
        } else {}
    }
}

void MiddleCode::programEnd() {
    string stmt = "\n";
    mipsStmts.emplace_back(stmt);
    stmt = "programEnd:";
    mipsStmts.emplace_back(stmt);
    stmt = "li $v0, 10";
    mipsStmts.emplace_back(stmt);
    stmt = "syscall";
    mipsStmts.emplace_back(stmt);
}

void MiddleCode::outputMips() {
    ofstream outputFile;
    outputFile.open("mips.txt");
    vector<string>::iterator it;
    for (it = mipsStmts.begin(); it != mipsStmts.end(); it++) {
        outputFile << (*it) << endl;
    }
}

void MiddleCode::outputMiddleCode()
{
    int index = 0;
    vector<string>::iterator it1;
    for (it1 = strs.begin(); it1 != strs.end(); it1++) {
        cout << "string" << index << ": " << (*it1)  << endl;
        index++;
    }
    cout << endl;
    vector<MiddleCodeItem>::iterator it2;
    for (it2 = middleCodeItems.begin(); it2 != middleCodeItems.end(); it2++) {
        switch ((*it2).op) {
            case Add:
                cout << "Add" << " " << (*it2).id1.name << " " << (*it2).id2.name << " " << (*it2).id3.name << endl;
                break;
            case Minus:
                cout << "Minus" << " " << (*it2).id1.name << " " << (*it2).id2.name << " " << (*it2).id3.name << endl;
                break;
            case Mul:
                cout << "Mul" << " " << (*it2).id1.name << " " << (*it2).id2.name << " " << (*it2).id3.name << endl;
                break;
            case Div:
                cout << "Div" << " " << (*it2).id1.name << " " << (*it2).id2.name << " " << (*it2).id3.name << endl;
                break;
            case Mod:
                cout << "Mod" << " " << (*it2).id1.name << " " << (*it2).id2.name << " " << (*it2).id3.name << endl;
                break;
            case GetInt:
                cout << "GetInt" << " " << (*it2).id1.name << endl;
                break;
            case Printf: {
                Symbol symbol1 = (*it2).id1;
                Symbol symbol2 = (*it2).id2;
                cout << "Printf" << " " << (*it2).id2.name ;
                if ((*it2).id1.name == "str") {
                    cout << (*it2).id2.value << endl;
                } else if ((*it2).id1.name == "expr") {
                    cout << " (" << (*it2).id2.basetype << ")" << endl;
                } else {}
                break;
            }
            case Declare: {
                cout << "Declare" << " " << (*it2).id1.name << " (" << (*it2).id1.datatype << " " << (*it2).id1.basetype << ")";
                if ((*it2).id1.dim > 0) {
                    cout << " (" << (*it2).id1.dim1 << " " << (*it2).id1.dim2 << ")" << endl;
                } else {
                    cout << endl;
                }
                break;
            }
            case Assign:
                cout << "Assign" << " " << (*it2).id1.name << " ("  << (*it2).id1.basetype  << ")" << " " << (*it2).id2.name << " ("  << (*it2).id2.basetype  << ")" << endl;
                break;
            case ArrayGet:
                cout << "ArrayGet" << " " <<(*it2).id1.name << " " << (*it2).id2.name << " " << (*it2).id3.name << endl;
                break;
            case ArrayAssign:
                cout << "ArrayAssign" << " " <<(*it2).id1.name << " " << (*it2).id2.name << " " << (*it2).id3.name << endl;
                break;
            case FuncDeclare: {
                cout << endl << "FuncDeclare" << " " << (*it2).id1.name;
                cout << " (params: " ;
                vector<Symbol>::iterator it3;
                for (it3 = (*it2).id1.params.begin(); it3 != (*it2).id1.params.end(); it3++) {
                    cout << (*it3).name << " ";
                }
                cout << " )" << endl;
                break;
            }
            case FuncCall:
                cout << "FuncCall" << " " << (*it2).id1.name << " " << (*it2).id2.name << endl;
                break;
            case FuncReturn:
                cout << "FuncReturn" << " " << (*it2).id1.name << endl;
                break;
            case PushParam:
                cout << "PushParam" << " " << (*it2).id1.name << endl;
                break;
            case BlockBegin:
                cout << "BlockBegin" << endl;
                break;
            case BlockEnd:
                cout << "BlockEnd" << endl;
                break;
            case Bge:
                cout << "Bge" << " " << (*it2).id1.name << " " << (*it2).id2.name << " " << (*it2).id3.name << endl;
                break;
            case Bgt:
                cout << "Bgt" << " " << (*it2).id1.name << " " << (*it2).id2.name << " " << (*it2).id3.name << endl;
                break;
            case Ble:
                cout << "Ble" << " " << (*it2).id1.name << " " << (*it2).id2.name << " " << (*it2).id3.name << endl;
                break;
            case Blt:
                cout << "Blt" << " " << (*it2).id1.name << " " << (*it2).id2.name << " " << (*it2).id3.name << endl;
                break;
            case Beq:
                cout << "Beq" << " " << (*it2).id1.name << " " << (*it2).id2.name << " " << (*it2).id3.name << endl;
                break;
            case Bne:
                cout << "Bne" << " " << (*it2).id1.name << " " << (*it2).id2.name << " " << (*it2).id3.name << endl;
                break;
            case Jump:
                cout << "Jump" << " " << (*it2).id1.name << endl;
                break;
            case Label:
                cout << "LABEL" << " " << (*it2).id1.name << endl;
                break;
            case ArrayGetArray:
                cout << "ArrayGetArray" << " " <<(*it2).id1.name << " " << (*it2).id2.name << " " << (*it2).id3.name << endl;
                break;
            case Lss:
                cout << "Lss" << " " << (*it2).id1.name << " " << (*it2).id2.name << " " << (*it2).id3.name << endl;
                break;
            case Leq:
                cout << "Leq" << " " << (*it2).id1.name << " " << (*it2).id2.name << " " << (*it2).id3.name << endl;
                break;
            case Gre:
                cout << "Gre" << " " << (*it2).id1.name << " " << (*it2).id2.name << " " << (*it2).id3.name << endl;
                break;
            case Geq:
                cout << "Geq" << " " << (*it2).id1.name << " " << (*it2).id2.name << " " << (*it2).id3.name << endl;
                break;
            case Eql:
                cout << "Eql" << " " << (*it2).id1.name << " " << (*it2).id2.name << " " << (*it2).id3.name << endl;
                break;
            case Neq:
                cout << "Neq" << " " << (*it2).id1.name << " " << (*it2).id2.name << " " << (*it2).id3.name << endl;
                break;
            case Not:
                cout << "Not" << " " << (*it2).id1.name << " " << (*it2).id2.name << endl;
                break;
            case And:
                cout << "And" << " " << (*it2).id1.name << " " << (*it2).id2.name << " " << (*it2).id3.name << endl;
                break;
            case Or:
                cout << "Or" << " " << (*it2).id1.name << " " << (*it2).id2.name << " " << (*it2).id3.name << endl;
                break;
        }
    }
}
