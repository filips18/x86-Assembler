#ifndef _H_SYMBOL_H
#define _H_SYMBOL_H

#include <string>
#include "Section.h"

struct forwardRefEntry {
    bool byte;
    unsigned long address;
    Section* section;
};


class Symbol {
    public:
        std::string name;
        unsigned long offset;
        std::string sectionName;
        unsigned sectionNumber;
        unsigned id;
        bool global;
        bool defined;
        std::vector<forwardRefEntry> forwardRefTable = std::vector<forwardRefEntry>();
};
#endif