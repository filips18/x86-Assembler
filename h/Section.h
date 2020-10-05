#ifndef _H_SECTION_H
#define _H_SECTION_H

#include <string>
#include "Relocation.h"
class Section {
    public:
        std::string name;
        unsigned size;
        unsigned address;
        std::string content = "";
        std::vector<Relocation> relTable = std::vector<Relocation>();
        unsigned number;
};

#endif