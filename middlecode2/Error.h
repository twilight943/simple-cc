#ifndef WITHERROR_ERROR_H
#define WITHERROR_ERROR_H

#include <iostream>
#include <fstream>
#include <vector>

using namespace std;

struct errorInfo {
    errorInfo(int line, string type);
    int line;
    string type;
};

class Error {
private:
    ofstream errorOutputFile;
    int errorNum{};
    vector<errorInfo> errorInfos;
    static bool linesort(const errorInfo& e1,const errorInfo& e2);

public:
    explicit Error(ofstream &output);
    Error();
    bool hasError() const;
    void outputError(int line, const string& errorcode);
    void sortAndOutput();
};

#endif //WITHERROR_ERROR_H
