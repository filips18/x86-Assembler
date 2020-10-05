#ifndef _H_RELOCATION_H
#define _H_RELOCATION_H

#include <string>

class Relocation {
    public:
        std::string type;
        unsigned long offset;
        std::string section;
        unsigned id;
};

class EQUPair {
    public:
        std::string symbolName;
        std::string section;
        long value;
};


class EQUTableRow {
    public:
        std::string symbol;
        std::vector<EQUPair> table = std::vector<EQUPair>();
        //std::vector<std::string> forwardRef;
        unsigned value;
};

#endif//.equ equSimbol, data1 + data2 - simbol3 // data1, data, 1
                                                // data2, data, 1
                                                // simbol3, _ , -1