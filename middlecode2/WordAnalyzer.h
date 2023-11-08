#ifndef WORDANALYZER_H
#define WORDANALYZER_H

#include <fstream>
#include <vector>

using namespace std;

struct Word {
    string code;
    string content;
    int line;
};

class WordAnalyzer {
private:
    ifstream inputfile;
    string filestr;
    char c;
    int charcount;
    int line;
    vector<Word> words;
    Word word(string code, string content);

public:
    explicit WordAnalyzer(ifstream &input);
    WordAnalyzer();
    Word analyze();
    int getPos();
    void setPos(int pos);
    int getLine();
    void setLine(int l);
};


#endif //WORDANALYZER_H
