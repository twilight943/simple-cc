#include <iostream>
#include "SyntaxAnalyzer.h"

using namespace std;

SyntaxAnalyzer::SyntaxAnalyzer(WordAnalyzer &newWordAnalyzer, Error &newError, SymbolTable &newSymbolTable, MiddleCode &newMiddleCode) {
    wordAnalyzer = move(newWordAnalyzer);
    error = move(newError);
    symbolTable = move(newSymbolTable);
    middleCode = move(newMiddleCode);
    whilenum = 0;
    tmpIndex = 0;
    tmpLableId = 0;
    ifId = 0;
    whileId = 0;
    nowFuncRet = NOBASETYPE;
}

void SyntaxAnalyzer::output(const std::string& syntaxComponent) {
    syntaxcom.emplace_back(syntaxComponent);
    //std::cout << syntaxComponent << std::endl;
    //outputfile << syntaxComponent << std::endl;
}

void SyntaxAnalyzer::output(const Word& outputword) {
    //cout << outputword.code << " " << outputword.content << endl;
    //outputfile << outputword.code << " " << outputword.content << endl;
}

void SyntaxAnalyzer::getWord() {
    lastword = word[0];
    word[0] = word[1];
    word[1] = word[2];
    word[2] = wordAnalyzer.analyze();
}

bool SyntaxAnalyzer::isUnaryExp(const Word& w) {
    if (w.code == "PLUS" || w.code == "MINU" || w.code == "IDENFR" ||
    w.code == "LPARENT" || w.code == "INTCON" || w.code == "NOT") {
        return true;
    }
    return false;
}

bool SyntaxAnalyzer::isStmt(const Word& w) {
    if (w.code == "IDENFR" || isUnaryExp(w) || w.code == "SEMICN" || w.code == "LBRACE" ||
        w.code == "IFTK" || w.code == "WHILETK" || w.code == "BREAKTK" || w.code == "CONTINUETK" ||
        w.code == "RETURNTK" || w.code == "PRINTFTK") {
        return true;
    }
    return false;
}

int SyntaxAnalyzer::expNumInString(const string& content) {
    int count = 0;
    for (size_t i = 0; (i = content.find("%d", i)) != string::npos; count++, i++);
    return count;
}

string SyntaxAnalyzer::checkString() {
    string content = word[0].content;
    string ret = content.substr(1,content.length() - 2);
    for(int i = 1; i < content.length() - 1; i++) {
        if (content[i] == 92) {
            if (content[i + 1] != 'n' || i + 1 >= content.length() - 1) {
                error.outputError(word[0].line,"a");
                break;
            } else {
                i++;
            }
        } else if (content[i] == '%') {
            if (content[i + 1] != 'd') {
                error.outputError(word[0].line,"a");
                break;
            } else {
                i++;
            }
        } else if (content[i] == 32 || content[i] == 33 || (content[i] >= 40 && content[i] <= 126 && content[i] != 92)) {
            continue;
        } else {
            error.outputError(word[0].line,"a");
        }
    }
    return ret;
}

void SyntaxAnalyzer::addToMap(const string& id, const string& type) {
    idMap.insert(pair<std::string, std::string>(id, type));
}

void SyntaxAnalyzer::analyze() {
    outputfile.open("output.txt");
    word[0] = wordAnalyzer.analyze();
    word[1] = wordAnalyzer.analyze();
    word[2] = wordAnalyzer.analyze();
    CompUnit();
    middleCode.handleAll();
    outputfile.close();
}

//编译单元 CompUnit → {Decl} {FuncDef} MainFuncDef
void SyntaxAnalyzer::CompUnit() {
    while (word[0].code == "CONSTTK" || ((word[0].code == "INTTK" || word[0].code == "CHARTK" || word[0].code == "AUTOTK") && word[2].code != "LPARENT")) {
        Decl();
    }
    while ((word[0].code == "INTTK" && word[1].code == "IDENFR") || word[0].code == "VOIDTK") {
        FuncDef();
    }
    if (word[0].code == "INTTK" && word[1].code == "MAINTK") {
        MainFuncDef();
    }
    output("<CompUnit>");
}

//声明 Decl → ConstDecl | VarDecl
void SyntaxAnalyzer::Decl() {
    if (word[0].code == "CONSTTK") {
        ConstDecl();
    } else if (word[0].code == "INTTK") {
        VarDecl();
    } else {}
}

//常量声明 ConstDecl → 'const' BType ConstDef { ',' ConstDef } ';'
void SyntaxAnalyzer::ConstDecl() {
    baseType bt;
    if (word[0].code == "CONSTTK") {
        output(word[0]);
        getWord();
        bt = BType();
        ConstDef(bt);
    }
    while (word[0].code == "COMMA") {
        output(word[0]);
        getWord();
        ConstDef(bt);
    }
    if (word[0].code == "SEMICN") {
        output(word[0]);
        getWord();
    } else {
        error.outputError(lastword.line,"i");
    }
    output("<ConstDecl>");
}

//基本类型 BType → 'int'
baseType SyntaxAnalyzer::BType() {
    if (word[0].code == "INTTK") {
        output(word[0]);
        getWord();
        return INT;
    } else if (word[0].code == "CHARTK") {
        output(word[0]);
        getWord();
        return CHAR;
    } else if (word[0].code == "AUTOTK") {
        output(word[0]);
        getWord();
        return AUTO;
    } else {}
    return NOBASETYPE;
}

//常数定义 ConstDef → Ident { '[' ConstExp ']' } '=' ConstInitVal
void SyntaxAnalyzer::ConstDef(baseType bt) {
    if (word[0].code == "IDENFR") {
        addToMap(word[0].content,"INT");
        if (!symbolTable.insert(word[0].content, CONST, bt)) {
            error.outputError(word[0].line,"b");
            output(word[0]);
            getWord();
            if (word[0].code == "LBRACK") {
                output(word[0]);
                getWord();
                ConstExp();
                if (word[0].code == "RBRACK") {
                    output(word[0]);
                    getWord();
                } else {
                    error.outputError(lastword.line,"k");
                }
                if (word[0].code == "LBRACK") {
                    output(word[0]);
                    getWord();
                    ConstExp();
                    if (word[0].code == "RBRACK") {
                        output(word[0]);
                        getWord();
                    } else {
                        error.outputError(lastword.line,"k");
                    }
                }
            }
            if (word[0].code == "ASSIGN") {
                output(word[0]);
                getWord();
                ConstInitVal();
            }
        } else {
            output(word[0]);
            getWord();
            int dimnum = 0, dim1 = 1,dim2 = 1;
            if (word[0].code == "LBRACK") {
                dimnum++;
                output(word[0]);
                getWord();
                dim1 = ConstExp();
                if (word[0].code == "RBRACK") {
                    output(word[0]);
                    getWord();
                } else {
                    error.outputError(lastword.line,"k");
                }
                if (word[0].code == "LBRACK") {
                    dimnum++;
                    output(word[0]);
                    getWord();
                    dim2 = ConstExp();
                    if (word[0].code == "RBRACK") {
                        output(word[0]);
                        getWord();
                    } else {
                        error.outputError(lastword.line,"k");
                    }
                }
            }
            symbolTable.changeDimType(dimnum,dim1,dim2);
            nowsymbol = symbolTable.getNowSymbol();
            middleCode.addMiddleCode(Declare, nowsymbol);
            Symbol symbol1 = nowsymbol;
            if (word[0].code == "ASSIGN") {
                output(word[0]);
                getWord();
                vector<int> values = ConstInitVal();
                symbolTable.changeConstValue(values);
                if (dimnum == 0) {
                    middleCode.addMiddleCode(Assign, symbol1, Symbol("CONSTINT", values[0]));
                } else {
                    for (int i = 0; i < values.size(); i++) {
                        middleCode.addMiddleCode(ArrayAssign, symbol1, Symbol("CONSTINT", i), Symbol("CONSTINT", values[i]));
                    }
                }
            }
        }
    }
    output("<ConstDef>");
}

//ConstInitVal → ConstExp | '{' [ ConstInitVal { ',' ConstInitVal } ] '}'
vector<int> SyntaxAnalyzer::ConstInitVal() {
    vector<int> ret;
    if (word[0].code == "LBRACE") {
        output(word[0]);
        getWord();
        if (word[0].code == "LBRACE" || isUnaryExp(word[0])) {
            vector<int> now = ConstInitVal();
            ret.insert(ret.end(),now.begin(),now.end());
            while (word[0].code == "COMMA") {
                output(word[0]);
                getWord();
                now = ConstInitVal();
                ret.insert(ret.end(),now.begin(),now.end());
            }
        }
        if (word[0].code == "RBRACE") {
            output(word[0]);
            getWord();
        }
    } else {
        ret.emplace_back(ConstExp());
    }
    output("<ConstInitVal>");
    return ret;
}

//VarDecl → BType VarDef { ',' VarDef } ';'
void SyntaxAnalyzer::VarDecl() {
    baseType bt = BType();
    VarDef(bt);
    while (word[0].code == "COMMA") {
        output(word[0]);
        getWord();
        VarDef(bt);
    }
    if (word[0].code == "SEMICN") {
        output(word[0]);
        getWord();
    } else {
        error.outputError(lastword.line,"i");
    }
    output("<VarDecl>");
}

//VarDef → Ident { '[' ConstExp ']' } | Ident { '[' ConstExp ']' } '=' InitVal
void SyntaxAnalyzer::VarDef(baseType bt) {
    if (word[0].code == "IDENFR") {
//        addToMap(word[0].content,"INT");
        if (!symbolTable.insert(word[0].content, VAR, bt)) {
            error.outputError(word[0].line,"b");
            output(word[0]);
            getWord();
            if (word[0].code == "LBRACK") {
                output(word[0]);
                getWord();
                ConstExp();
                if (word[0].code == "RBRACK") {
                    output(word[0]);
                    getWord();
                } else {
                    error.outputError(lastword.line,"k");
                }
                if (word[0].code == "LBRACK") {
                    output(word[0]);
                    getWord();
                    ConstExp();
                    if (word[0].code == "RBRACK") {
                        output(word[0]);
                        getWord();
                    } else {
                        error.outputError(lastword.line,"k");
                    }
                }
            }
            if (word[0].code == "ASSIGN") {
                output(word[0]);
                getWord();
                InitVal();
            }
        } else {
            Symbol s = symbolTable.getNowSymbol();
            output(word[0]);
            getWord();
            int dimnum = 0, dim1 = 1,dim2 = 1;
            if (word[0].code == "LBRACK") {
                dimnum++;
                output(word[0]);
                getWord();
                dim1 = ConstExp();
                if (word[0].code == "RBRACK") {
                    output(word[0]);
                    getWord();
                } else {
                    error.outputError(lastword.line,"k");
                }
                if (word[0].code == "LBRACK") {
                    dimnum++;
                    output(word[0]);
                    getWord();
                    dim2 = ConstExp();
                    if (word[0].code == "RBRACK") {
                        output(word[0]);
                        getWord();
                    } else {
                        error.outputError(lastword.line,"k");
                    }
                }
            }
            symbolTable.changeDimType(dimnum,dim1,dim2);
            nowsymbol = symbolTable.getNowSymbol();
            baseType nowbt = symbolTable.getNowBaseType();
            if(nowbt != AUTO && nowbt != NOBASETYPE) {
                middleCode.addMiddleCode(Declare, nowsymbol);
            }
            if (word[0].code == "ASSIGN") {
                output(word[0]);
                getWord();
                vector<Symbol> values = InitVal();
                if(nowbt == AUTO){
                    symbolTable.changeBasetype(values[0].basetype);
                    nowsymbol = symbolTable.getNowSymbol();
                    middleCode.addMiddleCode(Declare, nowsymbol);
                }
//                nowsymbol = symbolTable.getNowSymbol();
                Symbol symbol1 = nowsymbol;
                if (dimnum == 0) {
                    middleCode.addMiddleCode(Assign, symbol1, values[0]);
                } else {
                    for (int i = 0; i < values.size(); i++) {
                        middleCode.addMiddleCode(ArrayAssign, symbol1, Symbol("CONSTINT", i), values[i]);
                    }
                }
            }
        }
    }
    output("<VarDef>");
}

//InitVal → Exp | '{' [ InitVal { ',' InitVal } ] '}'
vector<Symbol> SyntaxAnalyzer::InitVal() {
    vector<Symbol> ret;
    if (word[0].code == "LBRACE") {
        output(word[0]);
        getWord();
        if (word[0].code == "LBRACE" || isUnaryExp(word[0])) {
            vector<Symbol> now = InitVal();
            ret.insert(ret.end(),now.begin(),now.end());
            while (word[0].code == "COMMA") {
                output(word[0]);
                getWord();
                now = InitVal();
                ret.insert(ret.end(),now.begin(),now.end());
            }
        }
        if (word[0].code == "RBRACE") {
            output(word[0]);
            getWord();
        }
    } else {
        Exp(0);
        ret.emplace_back(nowsymbol);
    }
    output("<InitVal>");
    return ret;
}

//FuncDef → FuncType Ident '(' [FuncFParams] ')' Block
void SyntaxAnalyzer::FuncDef() {
    if (word[0].code == "VOIDTK" || word[0].code == "INTTK") {
        baseType bt = FuncType();
        nowFuncRet = bt;
        if (word[0].code == "IDENFR") {
            if (!symbolTable.insert(word[0].content,FUNC,bt)) {
                error.outputError(word[0].line,"b");
            } else {
                Symbol nowfunc = symbolTable.findSymbol(word[0].content,FUNC);
            }
            if (bt == VOID) {
                addToMap(word[0].content, "NORETFUNC");
            } else {
                addToMap(word[0].content,"RETFUNC");
            }
            output(word[0]);
            getWord();
            if (word[0].code == "LPARENT") {
                output(word[0]);
                getWord();
                if (word[0].code == "INTTK") {
                    FuncFParams();
                }
                Symbol func = symbolTable.getNowFunc();
                middleCode.addMiddleCode(FuncDeclare, func);
                if (word[0].code == "RPARENT") {
                    output(word[0]);
                    getWord();
                } else {
                    error.outputError(lastword.line,"j");
                }
                if (word[0].code == "LBRACE") {
                    Block();
                    if (*(syntaxcom.end()-3) != "retstatement" && bt == INT) {
                        error.outputError(lastword.line,"g");
                    }
                }
            }
        }
        if (nowFuncRet == VOID) {
            middleCode.addMiddleCode(FuncReturn, Symbol("null", 0));
        }
    }
    symbolTable.clearNowFunc();
    output("<FuncDef>");
}

//MainFuncDef → 'int' 'main' '(' ')' Block
void SyntaxAnalyzer::MainFuncDef() {
    if (word[0].code == "INTTK") {
        baseType bt = INT;
        nowFuncRet = bt;
        output(word[0]);
        getWord();
        if (word[0].code == "MAINTK") {
            if (!symbolTable.insert(word[0].content,FUNC,bt)) {
                error.outputError(word[0].line, "b");
            }
            addToMap(word[0].content,"RETFUNC");
            output(word[0]);
            getWord();
            if (word[0].code == "LPARENT") {
                output(word[0]);
                getWord();
                if (word[0].code == "RPARENT") {
                    output(word[0]);
                    getWord();
                } else {
                    error.outputError(lastword.line,"j");
                }
                if (word[0].code == "LBRACE") {
                    middleCode.addMiddleCode(FuncDeclare, Symbol("main",FUNC,INT));
                    Block();
                    if (*(syntaxcom.end()-3) != "retstatement") {
                        error.outputError(lastword.line,"g");
                    }
                }
            } else {
                error.outputError(lastword.line,"j");
            }
        }
    }
    symbolTable.clearNowFunc();
    output("<MainFuncDef>");
}

//FuncType → 'void' | 'int'
baseType SyntaxAnalyzer::FuncType() {
    baseType bt = NOBASETYPE;
    if (word[0].code == "VOIDTK" || word[0].code == "INTTK") {
        if (word[0].code == "VOIDTK") {
            bt = VOID;
        } else {
            bt = INT;
        }
        output(word[0]);
        getWord();
    }
    output("<FuncType>");
    return bt;
}

//FuncFParams → FuncFParam { ',' FuncFParam }
void SyntaxAnalyzer::FuncFParams() {
    if (word[0].code == "INTTK") {
        FuncFParam();
    }
    while (word[0].code == "COMMA") {
        output(word[0]);
        getWord();
        if (word[0].code == "INTTK") {
            FuncFParam();
        }
    }
    output("<FuncFParams>");
}

//FuncFParam → BType Ident ['[' ']' { '[' ConstExp ']' }]
void SyntaxAnalyzer::FuncFParam() {
    if (word[0].code == "INTTK") {
        baseType bt = BType();
        if (word[0].code == "IDENFR") {
            string iden = word[0].content;
            int idenline = word[0].line;
            output(word[0]);
            getWord();
            int dimnum = 0;
            int dim2 = 1;
            if (word[0].code == "LBRACK") {
                dimnum++;
                output(word[0]);
                getWord();
                if (word[0].code == "RBRACK") {
                    output(word[0]);
                    getWord();
                } else {
                    error.outputError(lastword.line,"k");
                }
                if (word[0].code == "LBRACK") {
                    dimnum++;
                    output(word[0]);
                    getWord();
                    dim2 = ConstExp();
                    if (word[0].code == "RBRACK") {
                        output(word[0]);
                        getWord();
                    } else {
                        error.outputError(lastword.line,"k");
                    }
                }
            }
            if (!symbolTable.insert(iden, PARAM, bt, dimnum, dim2)) {
                error.outputError(idenline,"b");
            }
        }
    }
    output("<FuncFParam>");
}

//Block → '{' { BlockItem } '}'
void SyntaxAnalyzer::Block() {
    if (word[0].code == "LBRACE") {
        output(word[0]);
        getWord();
        symbolTable.addNewLocalTable();
        middleCode.addMiddleCode(BlockBegin);
        while (word[0].code != "RBRACE") {
            BlockItem();
        }
        if (word[0].code == "RBRACE") {
            output(word[0]);
            getWord();
        }
        middleCode.addMiddleCode(BlockEnd);
        symbolTable.removeNowLocalTable();
    }
    output("<Block>");
}

//BlockItem → Decl | Stmt
void SyntaxAnalyzer::BlockItem() {
    if (word[0].code == "CONSTTK" || (word[0].code == "INTTK" && word[2].code != "LPARENT")) {
        Decl();
    } else {
        Stmt();
    }
}

//Stmt → LVal '=' Exp ';' // 每种类型的语句都要覆盖
// | LVal = 'getint''('')'';'
// | [Exp] ';' //有⽆Exp两种情况
// | Block
// | 'if' '(' Cond ')' Stmt [ 'else' Stmt ] // 1.有else 2.⽆else
// | 'while' '(' Cond ')' Stmt
// | 'break' ';' | 'continue' ';'
// | 'return' [Exp] ';' // 1.有Exp 2.⽆Exp
// | 'printf''('FormatString{,Exp}')'';' // 1.有Exp 2.⽆Exp
void SyntaxAnalyzer::Stmt() {
    if (word[0].code == "SEMICN") {
        output(word[0]);
        getWord();
    } else if (word[0].code == "LBRACE") {
        Block();
    } else if (word[0].code == "IFTK") {
        ifId++;
        int nowifid = ifId;
        output(word[0]);
        getWord();
        if (word[0].code == "LPARENT") {
            output(word[0]);
            getWord();
            if (isUnaryExp(word[0])) {
                Cond(nowifid, "if");
                if (word[0].code == "RPARENT") {
                    output(word[0]);
                    getWord();
                } else {
                    error.outputError(lastword.line,"j");
                }
                Symbol ifbegin("if" + to_string(nowifid) + "begin", 0);
                Symbol ifelse("if" + to_string(nowifid) + "else", 0);
                Symbol ifend("if" + to_string(nowifid) + "end", 0);
                middleCode.addMiddleCode(Jump, ifelse);
                middleCode.addMiddleCode(Label, ifbegin);
                if (isStmt(word[0])) {
                    Stmt();
                }
                middleCode.addMiddleCode(Jump, ifend);
                middleCode.addMiddleCode(Label, ifelse);
                if (word[0].code == "ELSETK") {
                    output(word[0]);
                    getWord();
                    if (isStmt(word[0])) {
                        Stmt();
                    }
                }
                middleCode.addMiddleCode(Jump, ifend);
                middleCode.addMiddleCode(Label, ifend);
            }
        }
    } else if (word[0].code == "WHILETK") {
        output(word[0]);
        getWord();
        whilenum++;
        whileId++;
        int nowwhileid = whileId;
        whileIdStack.emplace(nowwhileid);
        Symbol whilehead("while" + to_string(nowwhileid) + "head", 0);
        Symbol whilebegin("while" + to_string(nowwhileid) + "begin", 0);
        Symbol whileend("while" + to_string(nowwhileid) + "end", 0);
        middleCode.addMiddleCode(Label, whilehead);
        if (word[0].code == "LPARENT") {
            output(word[0]);
            getWord();
            if (isUnaryExp(word[0])) {
                Cond(nowwhileid, "while");
                if (word[0].code == "RPARENT") {
                    output(word[0]);
                    getWord();
                } else {
                    error.outputError(lastword.line,"j");
                }
                middleCode.addMiddleCode(Jump, whileend);
                middleCode.addMiddleCode(Label, whilebegin);
                if (isStmt(word[0])) {
                    Stmt();
                }
                middleCode.addMiddleCode(Jump, whilehead);
            }
        }
        middleCode.addMiddleCode(Label, whileend);
        whileIdStack.pop();
        whilenum--;
    } else if (word[0].code == "BREAKTK") {
        int nowwhileid = whileIdStack.top();
        if (whilenum == 0) {
            error.outputError(word[0].line,"m");
        }
        output(word[0]);
        getWord();
        Symbol whileend("while" + to_string(nowwhileid) + "end", 0);
        middleCode.addMiddleCode(Jump, whileend);
        if (word[0].code == "SEMICN") {
            output(word[0]);
            getWord();
        } else {
            error.outputError(lastword.line,"i");
        }
    } else if (word[0].code == "CONTINUETK") {
        int nowwhileid = whileIdStack.top();
        if (whilenum == 0) {
            error.outputError(word[0].line,"m");
        }
        output(word[0]);
        getWord();
        Symbol whilehead("while" + to_string(nowwhileid) + "head", 0);
        middleCode.addMiddleCode(Jump, whilehead);
        if (word[0].code == "SEMICN") {
            output(word[0]);
            getWord();
        } else {
            error.outputError(lastword.line,"i");
        }
    } else if (word[0].code == "RETURNTK") {
        Symbol retSymbol("null", 0);
        Word tmpword = word[0];
        Symvalue newsym{};
        newsym.basetype = VOID;
        output(word[0]);
        getWord();
        if (isUnaryExp(word[0])) {
            newsym = Exp(0);
            retSymbol = nowsymbol;
        }
        if (nowFuncRet == INT) {
            middleCode.addMiddleCode(FuncReturn, retSymbol);
        } else {
            middleCode.addMiddleCode(FuncReturn, Symbol("null", 0));
        }
        if (nowFuncRet != newsym.basetype) {
            error.outputError(tmpword.line,"f");
        }
        if (nowFuncRet == newsym.basetype) {
            syntaxcom.emplace_back("retstatement");
        }
        if (word[0].code == "SEMICN") {
            output(word[0]);
            getWord();
        } else {
            error.outputError(lastword.line,"i");
        }
    } else if (word[0].code == "PRINTFTK") {
        Word tmpw = word[0];
        output(word[0]);
        getWord();
        if (word[0].code == "LPARENT") {
            output(word[0]);
            getWord();
            if (word[0].code == "STRCON") {
                string content = checkString();
                int expnum = expNumInString(content);
                output(word[0]);
                getWord();
                vector<Symbol> exprs;
                while (word[0].code == "COMMA") {
                    output(word[0]);
                    getWord();
                    if (isUnaryExp(word[0])) {
                        Exp(0);
                        exprs.push_back(nowsymbol);
                        expnum--;
                    }
                }
                if (expnum != 0) {
                    error.outputError(tmpw.line, "l");
                } else {
                    int stringIndex = 0;
                    int exprnumber = 0;
                    for (int nextIndex = 0; nextIndex < content.length();) {
                        nextIndex = (int) content.find("%d", stringIndex);
                        if (nextIndex == string::npos) {
                            break;
                        }
                        if (stringIndex < nextIndex) {
                            string str = content.substr(stringIndex, nextIndex - stringIndex);
                            int index = middleCode.addStr(str);
                            middleCode.addMiddleCode(Printf, Symbol("str", index), Symbol("str", index));
                        }
                        Symbol expr = exprs[exprnumber];
                        middleCode.addMiddleCode(Printf, Symbol("expr", 0), expr);
                        exprnumber++;
                        stringIndex = (int) nextIndex + 2;
                    }
                    if (stringIndex < content.length()) {
                        string str = content.substr(stringIndex, content.length());
                        int index = middleCode.addStr(str);
                        middleCode.addMiddleCode(Printf, Symbol("str", index), Symbol("str", index));
                    }
                }
                if (word[0].code == "RPARENT") {
                    output(word[0]);
                    getWord();
                } else {
                    error.outputError(lastword.line,"j");
                }
                if (word[0].code == "SEMICN") {
                    output(word[0]);
                    getWord();
                } else {
                    error.outputError(lastword.line,"i");
                }
            }
        }
    } else if (word[0].code == "IDENFR" && word[1].code != "LPARENT" && checkEq()) {
        Word lvaliden = word[0];
        LVal(0);
        Symbol symbol1 = nowsymbol;
        if (word[0].code == "ASSIGN") {
            if (symbolTable.findDTInAllFunc(lvaliden.content,CONST) == CONST) {
                error.outputError(lvaliden.line,"h");
            }
            output(word[0]);
            getWord();
            if (isUnaryExp(word[0])) {
                Exp(0);
                Symbol symbol2 = nowsymbol;
                if (symbol1.params.empty()) {
                    middleCode.addMiddleCode(Assign, symbol1, symbol2);
                } else {
                    middleCode.addMiddleCode(ArrayAssign, symbol1.params[0], symbol1.params[1], symbol2);
                }
                if (word[0].code == "SEMICN") {
                    output(word[0]);
                    getWord();
                } else {
                    error.outputError(lastword.line,"i");
                }
            } else if (word[0].code == "GETINTTK") {
                output(word[0]);
                getWord();
                if (word[0].code == "LPARENT") {
                    output(word[0]);
                    getWord();
                    if (word[0].code == "RPARENT") {
                        output(word[0]);
                        getWord();
                    } else {
                        error.outputError(lastword.line,"j");
                    }
                    Symbol symbol2(newTmpName(), 0);
                    middleCode.addMiddleCode(GetInt, symbol2);
                    if (symbol1.params.empty()) {
                        middleCode.addMiddleCode(Assign, symbol1, symbol2);
                    } else {
                        middleCode.addMiddleCode(ArrayAssign, symbol1.params[0], symbol1.params[1], symbol2);
                    }
                    if (word[0].code == "SEMICN") {
                        output(word[0]);
                        getWord();
                    } else {
                        error.outputError(lastword.line,"i");
                    }
                }
            }
        }
    } else if (isUnaryExp(word[0])) {
        Exp(0);
        if (word[0].code == "SEMICN") {
            output(word[0]);
            getWord();
        } else {
            error.outputError(lastword.line,"i");
        }
    } else {}
    output("<Stmt>");
}

//表达式 Exp → AddExp
Symvalue SyntaxAnalyzer::Exp(int mode) {
    Symvalue ret{};
    if (isUnaryExp(word[0])) {
        ret = AddExp(mode);
    }
    output("<Exp>");
    return ret;
}

//条件表达式 Cond → LOrExp
void SyntaxAnalyzer::Cond(int nowid, const string& type) {
    if (isUnaryExp(word[0])) {
        LOrExp(nowid, type);
    }
    output("<Cond>");
}

//左值表达式 LVal → Ident {'[' Exp ']'}
Symvalue SyntaxAnalyzer::LVal(int mode) {
    Symvalue ret{};
    if (word[0].code == "IDENFR") {
        string iden = word[0].content;
        Symbol idensym;
        if (symbolTable.findDTInAllFunc(iden,VAR) == NODATATYPE) {
            error.outputError(word[0].line,"c");
        } else {
            idensym = symbolTable.findSymbol(word[0].content,NOFUNC);
            nowsymbol = idensym;
            ret.basetype = idensym.basetype;
            ret.dim = idensym.dim;
        }
        output(word[0]);
        getWord();
        Symbol dim1Sym("CONSTINT", 0);
        Symbol dim2Sym("CONSTINT", 0);
        if (mode == 0) {
            if (word[0].code == "LBRACK") {
                if (ret.dim > 0) {
                    ret.dim--;
                }
                output(word[0]);
                getWord();
                Exp(mode);
                dim1Sym = nowsymbol;
                if (word[0].code == "RBRACK") {
                    output(word[0]);
                    getWord();
                } else {
                    error.outputError(lastword.line,"k");
                }
                if (word[0].code == "LBRACK") {
                    if (ret.dim > 0) {
                        ret.dim--;
                    }
                    output(word[0]);
                    getWord();
                    Exp(mode);
                    dim2Sym = nowsymbol;
                    if (word[0].code == "RBRACK") {
                        output(word[0]);
                        getWord();
                    } else {
                        error.outputError(lastword.line,"k");
                    }
                }
            }
            if (ret.dim == 0) {
                if (idensym.dim == 1) {
                    Symbol tmpSym3(newTmpName(), VAR, idensym.basetype);
                    middleCode.addMiddleCode(ArrayGet, idensym, dim1Sym, tmpSym3);
                    tmpSym3.params.push_back(idensym);
                    tmpSym3.params.push_back(dim1Sym);
                    nowsymbol = tmpSym3;
                } else if (idensym.dim == 2) {
                    Symbol tmpSym3(newTmpName(), VAR, idensym.basetype);
                    Symbol arraydim2("CONSTINT", idensym.dim2);
                    Symbol tmpSym1(newTmpName(), VAR, INT);
                    middleCode.addMiddleCode(Mul, dim1Sym, arraydim2, tmpSym1);
                    Symbol tmpSym2(newTmpName(), VAR, INT);
                    middleCode.addMiddleCode(Add, tmpSym1, dim2Sym, tmpSym2);
                    middleCode.addMiddleCode(ArrayGet, idensym, tmpSym2, tmpSym3);
                    tmpSym3.params.push_back(idensym);
                    tmpSym3.params.push_back(tmpSym2);
                    nowsymbol = tmpSym3;
                } else {}
            } else {
                if (idensym.dim == 2 && ret.dim == 1) {
                    Symbol tmpSym3(newTmpName(), VAR, idensym.basetype);
                    tmpSym3.dim = ret.dim;
                    tmpSym3.pointer = true;
                    middleCode.addMiddleCode(ArrayGetArray, idensym, dim1Sym, tmpSym3);
                    nowsymbol = tmpSym3;
                } else {
                    Symbol tmpSym3(newTmpName(), VAR, idensym.basetype);
                    Symbol tmpSym1("CONSTINT", 0);
                    tmpSym3.dim = ret.dim;
                    tmpSym3.pointer = true;
                    middleCode.addMiddleCode(ArrayGetArray, idensym, tmpSym1, tmpSym3);
                    nowsymbol = tmpSym3;
                }
            }
        } else {
            if (word[0].code == "LBRACK") {
                if (ret.dim > 0) {
                    ret.dim--;
                }
                output(word[0]);
                getWord();
                int dim1 = Exp(mode).value;
                dim1Sym = nowsymbol;
                if (word[0].code == "RBRACK") {
                    output(word[0]);
                    getWord();
                } else {
                    error.outputError(lastword.line,"k");
                }
                if (word[0].code == "LBRACK") {
                    if (ret.dim > 0) {
                        ret.dim--;
                    }
                    output(word[0]);
                    getWord();
                    int dim2 = Exp(mode).value;
                    dim2Sym = nowsymbol;
                    if (word[0].code == "RBRACK") {
                        output(word[0]);
                        getWord();
                    } else {
                        error.outputError(lastword.line,"k");
                    }
                    if (idensym.datatype == CONST) {
                        int idendim1 = idensym.dim1;
                        int idendim2 = idensym.dim2;
                        ret.value = idensym.constvalue[dim1*idendim2+dim2];
                    }
                } else {
                    if (idensym.datatype == CONST) {
                        ret.value = idensym.constvalue[dim1];
                    }
                }
            } else {
                if (idensym.datatype == CONST) {
                    ret.value = idensym.constvalue[0];
                }
            }
            if (ret.dim == 0) {
                if (idensym.dim == 1) {
                    Symbol tmpSym3(newTmpName(), VAR, idensym.basetype);
                    middleCode.addMiddleCode(ArrayGet, idensym, dim1Sym, tmpSym3);
                    nowsymbol = tmpSym3;
                } else if (idensym.dim == 2) {
                    Symbol tmpSym3(newTmpName(), VAR, idensym.basetype);
                    Symbol arraydim2("CONSTINT", idensym.dim2);
                    Symbol tmpSym1(newTmpName(), VAR, INT);
                    middleCode.addMiddleCode(Mul, dim1Sym, arraydim2, tmpSym1);
                    Symbol tmpSym2(newTmpName(), VAR, INT);
                    middleCode.addMiddleCode(Add, tmpSym1, dim2Sym, tmpSym2);
                    middleCode.addMiddleCode(ArrayGet, idensym, tmpSym2, tmpSym3);
                    nowsymbol = tmpSym3;
                } else {}
            } else {
                if (idensym.dim == 2 && ret.dim == 1) {
                    Symbol tmpSym3(newTmpName(), VAR, idensym.basetype);
                    tmpSym3.dim = ret.dim;
                    tmpSym3.pointer = true;
                    middleCode.addMiddleCode(ArrayGetArray, idensym, dim1Sym, tmpSym3);
                    nowsymbol = tmpSym3;
                } else {
                    Symbol tmpSym3(newTmpName(), VAR, idensym.basetype);
                    Symbol tmpSym1("CONSTINT", 0);
                    tmpSym3.dim = ret.dim;
                    tmpSym3.pointer = true;
                    middleCode.addMiddleCode(ArrayGetArray, idensym, tmpSym1, tmpSym3);
                    nowsymbol = tmpSym3;
                }
            }
        }
    }
    output("<LVal>");
    return ret;
}

//PrimaryExp → '(' Exp ')' | LVal | Number
Symvalue SyntaxAnalyzer::PrimaryExp(int mode) {
    Symvalue ret{};
    if (word[0].code == "LPARENT") {
        output(word[0]);
        getWord();
        Symvalue sv = Exp(mode);
        ret.basetype = sv.basetype;
        ret.dim = sv.dim;
        if (mode == 1) {
            ret.value = sv.value;
        }
        if (word[0].code == "RPARENT") {
            output(word[0]);
            getWord();
        } else {
            error.outputError(lastword.line,"j");
        }
    } else if (word[0].code == "IDENFR") {
        Symvalue sv = LVal(mode);
        ret.basetype = sv.basetype;
        ret.dim = sv.dim;
        if (mode == 1) {
            ret.value = sv.value;
        }
    } else if (word[0].code == "INTCON") {
        int value = Number();
        Symbol tmpSym("CONSTINT", value);
        nowsymbol = tmpSym;
        ret.value = value;
        ret.basetype = INT;
        ret.dim = 0;
    } else if (word[0].code == "CHARCON") {
        int value = Number();
        Symbol tmpSym("CONSTCHAR", value);
        nowsymbol = tmpSym;
        ret.value = value;
        ret.basetype = CHAR;
        ret.dim = 0;
    } else {}
    output("<PrimaryExp>");
    return ret;
}

//Number → IntConst
int SyntaxAnalyzer::Number() {
    int result = 0;
    if (word[0].code == "INTCON") {
        result = stoi(word[0].content);
        output(word[0]);
        getWord();
    }
    output("<Number>");
    return result;
}

int SyntaxAnalyzer::Char() {
    int result = 0;
    if (word[0].code == "CHARCON") {
        result = int(word[0].content[0]);
        output(word[0]);
        getWord();
    }
    output("<Char>");
    return result;
}

//UnaryExp → PrimaryExp | Ident '(' [FuncRParams] ')' | UnaryOp UnaryExp
Symvalue SyntaxAnalyzer::UnaryExp(int mode) {
    Symvalue ret{};
    if (word[0].code == "LPARENT" || word[0].code == "INTCON" || word[0].code == "CHARCON" ||
    (word[0].code == "IDENFR" && word[1].code != "LPARENT")) {
        Symvalue sv = PrimaryExp(mode);
        ret.basetype = sv.basetype;
        ret.dim = sv.dim;
        if (mode == 1) {
            ret.value = sv.value;
        }
    } else if (word[0].code == "IDENFR" && word[1].code == "LPARENT") {
        Word funciden = word[0];
        string funcname = word[0].content;
        Symbol calledfunc;
        if (symbolTable.findDTInAllFunc(funcname,FUNC) == NODATATYPE) {
            error.outputError(word[0].line,"c");
        } else {
            calledfunc = symbolTable.findSymbol(funcname,FUNC);
            ret.basetype = calledfunc.basetype;
            ret.dim = 0;
        }
        output(word[0]);
        getWord();
        output(word[0]);
        getWord();
        vector<Symvalue> rParams;
        if (isUnaryExp(word[0])) {
            rParams = FuncRParams(funciden);
        }
        vector<Symbol> funcParams = symbolTable.getFuncParams(funcname);
        if (symbolTable.findDTInAllFunc(funcname,FUNC) != NODATATYPE) {
            if (rParams.size() != funcParams.size()) {
                error.outputError(funciden.line,"d");
            } else {
                std::vector<Symvalue>::iterator it;
                std::vector<Symbol>::iterator it2;
                for (it = rParams.begin(), it2 = funcParams.begin(); it != rParams.end() && it2 != funcParams.end(); it++, it2++) {
                    if ((*it).basetype != (*it2).basetype || (*it).dim != (*it2).dim) {
                        error.outputError(funciden.line, "e");
                    }
                }
            }
        }
        if (calledfunc.datatype == FUNC) {
            Symbol tmpSymbol(newTmpName(), VAR, calledfunc.basetype);
            middleCode.addMiddleCode(FuncCall, calledfunc, tmpSymbol);
            nowsymbol = tmpSymbol;
        }
        if (word[0].code == "RPARENT") {
            output(word[0]);
            getWord();
        } else {
            error.outputError(lastword.line,"j");
        }
    } else if (word[0].code == "PLUS" || word[0].code == "MINU" || word[0].code == "NOT") {
        int sign;
        if (word[0].code == "PLUS") {
            sign = 0;
        } else if (word[0].code == "MINU") {
            sign = 1;
        } else {
            sign = 2;
        }
        UnaryOp();
        Symvalue sv = UnaryExp(mode);
        Symbol symbol1 = nowsymbol;
        if (sign == 1) {
            Symbol tmpSymbol(newTmpName(), VAR, INT);
            middleCode.addMiddleCode(Mul, symbol1, Symbol("CONSTINT", -1), tmpSymbol);
            nowsymbol = tmpSymbol;
        } else if (sign == 0) {
            nowsymbol = symbol1;
        } else {
            Symbol tmpSymbol(newTmpName(), VAR, INT);
            middleCode.addMiddleCode(Not, symbol1, tmpSymbol);
            nowsymbol = tmpSymbol;
        }
        ret.basetype = sv.basetype;
        ret.dim = sv.dim;
        if (sign == 0) {
            ret.value = sv.value;
        } else if (sign == 1) {
            ret.value = -sv.value;
        } else {
            ret.value = 0;
        }
    }
    output("<UnaryExp>");
    return ret;
}

//UnaryOp → '+' | '−' | '!'
void SyntaxAnalyzer::UnaryOp() {
    if (word[0].code == "PLUS" || word[0].code == "MINU" || word[0].code == "NOT") {
        output(word[0]);
        getWord();
    }
    output("<UnaryOp>");
}

//FuncRParams → Exp { ',' Exp }
vector<Symvalue> SyntaxAnalyzer::FuncRParams(const Word& word1) {
    vector<Symvalue> realParamsType;
    string funcname = word1.content;
    if (isUnaryExp(word[0])) {
        Symvalue newsym = Exp(0);
        Symbol paramSym = nowsymbol;
        middleCode.addMiddleCode(PushParam, paramSym);
        realParamsType.emplace_back(newsym);
        while (word[0].code == "COMMA") {
            output(word[0]);
            getWord();
            newsym = Exp(0);
            paramSym = nowsymbol;
            middleCode.addMiddleCode(PushParam, paramSym);
            realParamsType.emplace_back(newsym);
        }
    }
    output("<FuncRParams>");
    return realParamsType;
}

//MulExp → UnaryExp | MulExp ('*' | '/' | '%') UnaryExp
Symvalue SyntaxAnalyzer::MulExp(int mode) {
    Symvalue ret{};
    if (isUnaryExp(word[0])) {
        Symvalue sv = UnaryExp(mode);
        Symbol symbol1 = nowsymbol;
        ret.basetype = sv.basetype;
        ret.dim = sv.dim;
        if (mode == 1) {
            ret.value = sv.value;
        }
        while (word[0].code == "MULT" || word[0].code == "DIV" || word[0].code == "MOD") {
            Operator op;
            if (word[0].code == "MULT") {
                op = Mul;
            } else if (word[0].code == "DIV") {
                op = Div;
            } else {
                op = Mod;
            }
            output("<MulExp>");
            output(word[0]);
            getWord();
            if (isUnaryExp(word[0])) {
                sv = UnaryExp(mode);
                Symbol symbol2 = nowsymbol;
                Symbol symbol3(newTmpName(), VAR, sv.basetype);
                middleCode.addMiddleCode(op, symbol1, symbol2, symbol3);
                symbol1 = symbol3;
                nowsymbol = symbol3;
                ret.basetype = (sv.basetype != NOBASETYPE) ? sv.basetype : ret.basetype;
                ret.dim = sv.dim;
                if (mode == 1) {
                    if (op == Mul) {
                        ret.value *= sv.value;
                    } else if (op == Div) {
                        ret.value /= sv.value;
                    } else {
                        ret.value %= sv.value;
                    }
                }
            }
        }
    }
    output("<MulExp>");
    return ret;
}

//AddExp → MulExp | AddExp ('+' | '−') MulExp
Symvalue SyntaxAnalyzer::AddExp(int mode) {
    Symvalue ret{};
    if (isUnaryExp(word[0])) {
        Symvalue sv = MulExp(mode);
        Symbol symbol1 = nowsymbol;
        ret.basetype = sv.basetype;
        ret.dim = sv.dim;
        if (mode == 1) {
            ret.value = sv.value;
        }
        while (word[0].code == "PLUS" || word[0].code == "MINU") {
            Operator op = (word[0].code == "PLUS") ? Add : Minus;
            output("<AddExp>");
            output(word[0]);
            getWord();
            if (isUnaryExp(word[0])) {
                sv = MulExp(mode);
                Symbol symbol2 = nowsymbol;
                Symbol symbol3(newTmpName(), VAR, sv.basetype);
                middleCode.addMiddleCode(op, symbol1, symbol2, symbol3);
                symbol1 = symbol3;
                nowsymbol = symbol3;
                ret.basetype = (sv.basetype != NOBASETYPE) ? sv.basetype : ret.basetype;
                ret.dim = sv.dim;
                if (mode == 1) {
                    if (op == Add) {
                        ret.value += sv.value;
                    } else if (op == Minus) {
                        ret.value -= sv.value;
                    } else {
                        ret.value = 0;
                    }
                }
            }
        }
    }
    output("<AddExp>");
    return ret;
}

//RelExp → AddExp | RelExp ('<' | '>' | '<=' | '>=') AddExp
void SyntaxAnalyzer::RelExp() {
    if (isUnaryExp(word[0])) {
        AddExp(0);
        Symbol symbol1 = nowsymbol;
        while (word[0].code == "LSS" || word[0].code == "LEQ" ||
        word[0].code == "GRE" || word[0].code == "GEQ") {
            Operator op;
            if (word[0].code == "LSS") {
                op = Lss;
            } else if (word[0].code == "LEQ") {
                op = Leq;
            } else if (word[0].code == "GRE") {
                op = Gre;
            } else {
                op = Geq;
            }
            output("<RelExp>");
            output(word[0]);
            getWord();
            if (isUnaryExp(word[0])) {
                AddExp(0);
                Symbol symbol2 = nowsymbol;
                Symbol symbol3(newTmpName(), VAR, INT);
                middleCode.addMiddleCode(op, symbol1, symbol2, symbol3);
                symbol1 = symbol3;
                nowsymbol = symbol3;
            }
        }
    }
    output("<RelExp>");
}

//EqExp → RelExp | EqExp ('==' | '!=') RelExp
void SyntaxAnalyzer::EqExp() {
    if (isUnaryExp(word[0])) {
        RelExp();
        Symbol symbol1 = nowsymbol;
        while (word[0].code == "EQL" || word[0].code == "NEQ") {
            Operator op;
            if (word[0].code == "EQL") {
                op = Eql;
            } else {
                op = Neq;
            }
            output("<EqExp>");
            output(word[0]);
            getWord();
            if (isUnaryExp(word[0])) {
                RelExp();
                Symbol symbol2 = nowsymbol;
                Symbol symbol3(newTmpName(), VAR, INT);
                middleCode.addMiddleCode(op, symbol1, symbol2, symbol3);
                symbol1 = symbol3;
                nowsymbol = symbol3;
            }
        }
    }
    output("<EqExp>");
}

//LAndExp → EqExp | LAndExp '&&' EqExp
void SyntaxAnalyzer::LAndExp(int tmpLabel) {
    if (isUnaryExp(word[0])) {
        EqExp();
        Symbol symbol1 = nowsymbol;
        while (word[0].code == "AND") {
            middleCode.addMiddleCode(Beq, symbol1, Symbol("label" + to_string(tmpLabel), 0));
            output("<LAndExp>");
            output(word[0]);
            getWord();
            if (isUnaryExp(word[0])) {
                EqExp();
                symbol1 = nowsymbol;
            }
        }
        middleCode.addMiddleCode(Beq, symbol1,Symbol("label" + to_string(tmpLabel), 0));
    }
    output("<LAndExp>");
}

//LOrExp → LAndExp | LOrExp '||' LAndExp
void SyntaxAnalyzer::LOrExp(int nowid, const string& type) {
    if (isUnaryExp(word[0])) {
        tmpLableId++;
        int nowtmpLabel = tmpLableId;
        LAndExp(nowtmpLabel);
        Symbol symbol1 = nowsymbol;
        while (word[0].code == "OR") {
            middleCode.addMiddleCode(Label, Symbol("label" + to_string(nowtmpLabel), 0));
            middleCode.addMiddleCode(Bne, symbol1,Symbol(type + to_string(nowid) + "begin", 0));
            output("<LOrExp>");
            output(word[0]);
            getWord();
            if (isUnaryExp(word[0])) {
                tmpLableId++;
                nowtmpLabel = tmpLableId;
                LAndExp(nowtmpLabel);
                symbol1 = nowsymbol;
            }
        }
        middleCode.addMiddleCode(Label, Symbol("label" + to_string(nowtmpLabel), 0));
        middleCode.addMiddleCode(Bne, symbol1,Symbol(type + to_string(nowid) + "begin", 0));
    }
    output("<LOrExp>");
}

//ConstExp → AddExp
int SyntaxAnalyzer::ConstExp() {
    int ret = 0;
    if (isUnaryExp(word[0])) {
        ret = AddExp(1).value;
    }
    output("<ConstExp>");
    return ret;
}

bool SyntaxAnalyzer::checkEq() {
    int pos = wordAnalyzer.getPos();
    int line = wordAnalyzer.getLine();
    bool ret = false;
    Word nowlastword = lastword;
    Word nowword0 = word[0];
    Word nowword1 = word[1];
    Word nowword2 = word[2];
    checkLval();
    if (word[0].code == "ASSIGN") {
        ret = true;
    }
    wordAnalyzer.setPos(pos);
    wordAnalyzer.setLine(line);
    lastword = nowlastword;
    word[0] = nowword0;
    word[1] = nowword1;
    word[2] = nowword2;
    return ret;
}

void SyntaxAnalyzer::checkLval() {
    if (word[0].code == "IDENFR") {
        getWord();
        while (word[0].code == "LBRACK") {
            getWord();
            checkExp();
            if (word[0].code == "RBRACK") {
                getWord();
            }
        }
    }
}

void SyntaxAnalyzer::checkExp() {
    if (isUnaryExp(word[0])) {
        checkMulExp();
        while (word[0].code == "PLUS" || word[0].code == "MINU") {
            getWord();
            if (isUnaryExp(word[0])) {
                checkMulExp();
            }
        }
    }
}

void SyntaxAnalyzer::checkMulExp() {
    if (isUnaryExp(word[0])) {
        checkUnaryExp();
        while (word[0].code == "MULT" || word[0].code == "DIV" || word[0].code == "MOD") {
            getWord();
            if (isUnaryExp(word[0])) {
                checkUnaryExp();
            }
        }
    }
}

void SyntaxAnalyzer::checkUnaryExp() {
    if (word[0].code == "LPARENT" || word[0].code == "INTCON" ||
        (word[0].code == "IDENFR" && word[1].code != "LPARENT")) {
        checkPrimaryExp();
    } else if (word[0].code == "IDENFR" && word[1].code == "LPARENT") {
        getWord();
        getWord();
        if (isUnaryExp(word[0])) {
            checkFuncRParams();
        }
        if (word[0].code == "RPARENT") {
            getWord();
        }
    } else if (word[0].code == "PLUS" || word[0].code == "MINU" || word[0].code == "NOT") {
        getWord();
        checkUnaryExp();
    }
}

void SyntaxAnalyzer::checkPrimaryExp() {
    if (word[0].code == "LPARENT") {
        getWord();
        checkExp();
        if (word[0].code == "RPARENT") {
            getWord();
        }
    } else if (word[0].code == "IDENFR") {
        checkLval();
    } else if (word[0].code == "INTCON") {
        getWord();
    } else {}
}

void SyntaxAnalyzer::checkFuncRParams() {
    if (isUnaryExp(word[0])) {
        checkExp();
        while (word[0].code == "COMMA") {
            getWord();
            checkExp();
        }
    }
}

string SyntaxAnalyzer::newTmpName() {
    string ret = "tmp" + to_string(tmpIndex);
    tmpIndex++;
    return ret;
}