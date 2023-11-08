#include <iostream>
#include <sstream>
#include "WordAnalyzer.h"

using namespace std;

WordAnalyzer::WordAnalyzer(ifstream &input) {
    inputfile = move(input);
    ostringstream buf;
    char ch;
    while (buf && inputfile.get(ch)) {
        buf.put(ch);
    }
    filestr = buf.str();
    charcount = 0;
    line = 1;
    c = '\0';
}

WordAnalyzer::WordAnalyzer() = default;

Word WordAnalyzer::word(string code, string content) {
    Word newword;
    newword.code = move(code);
    newword.content = move(content);
    newword.line = line;
    return newword;
}

Word WordAnalyzer::analyze() {
    string code,token;
    while (charcount < filestr.size()) {
        c = filestr.at(charcount);
        charcount++;
        if (c == '_' || isalpha(c)) {
            token.append(1, c);
            c = filestr.at(charcount);
            while (c == '_' || isdigit(c) || isalpha(c)) {
                charcount++;
                token.append(1, c);
                if (charcount >= filestr.size()) {
                    break;
                }
                c = filestr.at(charcount);
            }
            if (token == "main") {
                code = "MAINTK";
            } else if (token == "const") {
                code = "CONSTTK";
            } else if (token == "int") {
                code = "INTTK";
            } else if (token == "char") {
                code = "CHARTK";
            } else if (token == "auto") {
                code = "AUTOTK";
            } else if (token == "break") {
                code = "BREAKTK";
            } else if (token == "continue") {
                code = "CONTINUETK";
            } else if (token == "if") {
                code = "IFTK";
            } else if (token == "else") {
                code = "ELSETK";
            } else if (token == "while") {
                code = "WHILETK";
            } else if (token == "getint") {
                code = "GETINTTK";
            } else if (token == "printf") {
                code = "PRINTFTK";
            } else if (token == "return") {
                code = "RETURNTK";
            } else if (token == "void") {
                code = "VOIDTK";
            } else {
                code = "IDENFR";
            }
            words.push_back(word(code,token));
            return word(code,token);
        } else if (isdigit(c)) {
            token.append(1,c);
            if (c == '0') {
                c = filestr.at(charcount);
                if ((!isdigit(c)) && (!isalpha(c)) && (c != '_')) {
                    code = "INTCON";
                }
            } else {
                c = filestr.at(charcount);
                while(isdigit(c) && charcount < filestr.size()) {
                    charcount++;
                    token.append(1,c);
                    c = filestr.at(charcount);
                }
                code = "INTCON";
            }
            words.push_back(word(code,token));
            return word(code,token);
        } else if (c == '\'') {
//            token.append(1,c);
            charcount++;
            c = filestr.at(charcount);
//            while (c != '\'' && charcount < filestr.size()) {
//                charcount++;
//                token.append(1,c);
//                c = filestr.at(charcount);
//            }
            token.append(1,c);
            charcount++;
            charcount++;
            code = "CHARCON";
            words.push_back(word(code,token));
            return word(code,token);
        }  else if (c == '\"') {
            token.append(1,c);
            c = filestr.at(charcount);
            while (c != '\"' && charcount < filestr.size()) {
                charcount++;
                token.append(1,c);
                c = filestr.at(charcount);
            }
            charcount++;
            token.append(1,c);
            code = "STRCON";
            words.push_back(word(code,token));
            return word(code,token);
        } else if (c == '!') {
            token.append(1,c);
            c = filestr.at(charcount);
            if (c == '=') {
                charcount++;
                token.append(1,c);
                code = "NEQ";
            } else {
                code = "NOT";
            }
            words.push_back(word(code,token));
            return word(code,token);
        } else if (c == '=') {
            token.append(1,c);
            c = filestr.at(charcount);
            if (c == '=') {
                charcount++;
                token.append(1,c);
                code = "EQL";
            } else {
                code = "ASSIGN";
            }
            words.push_back(word(code,token));
            return word(code,token);
        } else if (c == '<') {
            token.append(1,c);
            c = filestr.at(charcount);
            if (c == '=') {
                charcount++;
                token.append(1,c);
                code = "LEQ";
            } else {
                code = "LSS";
            }
            words.push_back(word(code,token));
            return word(code,token);
        } else if (c == '>') {
            token.append(1,c);
            c = filestr.at(charcount);
            if (c == '=') {
                charcount++;
                token.append(1,c);
                code = "GEQ";
            } else {
                code = "GRE";
            }
            words.push_back(word(code,token));
            return word(code,token);
        } else if (c == '&') {
            token.append(1,c);
            c = filestr.at(charcount);
            if (c == '&') {
                charcount++;
                token.append(1,c);
                code = "AND";
            }
            words.push_back(word(code,token));
            return word(code,token);
        } else if (c == '|') {
            token.append(1,c);
            c = filestr.at(charcount);
            if (c == '|') {
                charcount++;
                token.append(1,c);
                code = "OR";
            }
            words.push_back(word(code,token));
            return word(code,token);
        } else if (c == '+') {
            token.append(1,c);
            code = "PLUS";
            words.push_back(word(code,token));
            return word(code,token);
        } else if (c == '-') {
            token.append(1,c);
            code = "MINU";
            words.push_back(word(code,token));
            return word(code,token);
        } else if (c == '*') {
            token.append(1,c);
            code = "MULT";
            words.push_back(word(code,token));
            return word(code,token);
        } else if (c == '/') {
            token.append(1,c);
            c = filestr.at(charcount);
            if (c == '/') {
                charcount++;
                c = filestr.at(charcount);
                while(c != '\n' && charcount < filestr.size()) {
                    charcount++;
                    c = filestr.at(charcount);
                }
                token.clear();
            } else if (c == '*') {
                charcount++;
                c = filestr.at(charcount);
                char peek = filestr.at(charcount + 1);
                while(charcount < filestr.size()) {
                    if (c == '*' && peek == '/') {
                        break;
                    }
                    charcount++;
                    c = filestr.at(charcount);
                    peek = filestr.at(charcount + 1);
                }
                charcount++;
                charcount++;
                token.clear();
            } else {
                code = "DIV";
                words.push_back(word(code,token));
                return word(code,token);
            }
        } else if (c == '%') {
            token.append(1,c);
            code = "MOD";
            words.push_back(word(code,token));
            return word(code,token);
        } else if (c == ';') {
            token.append(1,c);
            code = "SEMICN";
            words.push_back(word(code,token));
            return word(code,token);
        } else if (c == ',') {
            token.append(1,c);
            code = "COMMA";
            words.push_back(word(code,token));
            return word(code,token);
        } else if (c == '(') {
            token.append(1,c);
            code = "LPARENT";
            words.push_back(word(code,token));
            return word(code,token);
        } else if (c == ')') {
            token.append(1,c);
            code = "RPARENT";
            words.push_back(word(code,token));
            return word(code,token);
        } else if (c == '[') {
            token.append(1,c);
            code = "LBRACK";
            words.push_back(word(code,token));
            return word(code,token);
        } else if (c == ']') {
            token.append(1,c);
            code = "RBRACK";
            words.push_back(word(code,token));
            return word(code,token);
        } else if (c == '{') {
            token.append(1,c);
            code = "LBRACE";
            words.push_back(word(code,token));
            return word(code,token);
        } else if (c == '}') {
            token.append(1,c);
            code = "RBRACE";
            words.push_back(word(code,token));
            return word(code,token);
        } else if (c == '\n') {
            line++;
            continue;
        } else {
            continue;
        }
        c = '\0';
    }
    return word("EOF","EOF");
}

void WordAnalyzer::setPos(int pos) {
    charcount = pos;
}

int WordAnalyzer::getPos() {
    return charcount;
}

void WordAnalyzer::setLine(int l) {
    line = l;
}

int WordAnalyzer::getLine() {
    return line;
}
