#ifndef _H_ASSEMBLER_H
#define _H_ASSEMBLER_H

#include <fstream>
#include <vector>
#include <regex>
#include <cstdint>
#include <climits>

#include "EnumCollection.h"
#include "Symbol.h"
#include "Section.h"
#include "Relocation.h"

using namespace std;

class Assembler {
    private:
        states assemblerStatus;
        Section* currentSection = 0;
        unsigned long lineCounter = 1;
        unsigned long locationCounter = 0;
        unsigned numOfSimbols = 0;

        vector<Section> sections = vector<Section>();
        vector<Symbol> tableOfSymbols = vector<Symbol>();
        
        vector<string> globalSymbols = vector<string>();
        vector<string> externSymbols = vector<string>(); //mozda ne treba

        vector<EQUTableRow> EQUTable = vector<EQUTableRow>();
    
    public:

    Assembler();

    ~Assembler();

    void start(ifstream* input, ofstream* output);
    void firstPass(ifstream* in, ofstream* out);
    void stringToUpper(string* str);

    bool findSymbol(string symbol);
    void insertSymbol(Symbol symbol);
    void updateSymbol(Symbol symbol);
    Symbol* getSymbol(string name);
    bool checkGlobAndExtern(string symbol);
    bool checkExtern(string symbol);

    string binaryToHexa(char* bin, int maxNoOfHexDigits);
    string decimalToHexa(char* bin, int maxNoOfHexDigits);

    bool findEQUEntry(string symbol);

};

#endif