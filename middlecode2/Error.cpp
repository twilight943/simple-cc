#include "Error.h"
#include <iostream>
#include <algorithm>

using namespace std;

Error::Error(ofstream &output) {
    errorOutputFile = move(output);
    errorNum = 0;
}

Error::Error() = default;

errorInfo::errorInfo(int line, string type) : line(line), type(std::move(type)) {}

bool Error::hasError() const { return errorNum; }

void Error::outputError(int line, const string& errorcode) {
    errorInfos.emplace_back(line, errorcode);
    errorNum++;
}

void Error::sortAndOutput() {
    sort(errorInfos.begin(),errorInfos.end(), linesort);
    vector<errorInfo>::iterator it;
    //for (it = errorInfos.begin(); it != errorInfos.end(); it++) {
        //cout << (*it).line << " " << (*it).type << endl;
        //errorOutputFile << (*it).line << " " << (*it).type << endl;
    //}
}

bool Error::linesort(const errorInfo& e1, const errorInfo& e2) {
    return e1.line < e2.line;
}

