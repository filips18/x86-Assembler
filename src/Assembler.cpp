#include <cstring>
#include <algorithm>
#include <iostream>
#include <cmath>

#include "../h/Assembler.h"

Assembler::Assembler() {

}

Assembler::~Assembler() {

}


void Assembler::stringToUpper(string* str){
    for(int i = 0; i < str->length(); i++) (*str)[i] = toupper((*str)[i]);
}

bool Assembler::findSymbol(string s) {
    for(int i = 0; i < tableOfSymbols.size(); i++) {
        if(!strcmp((char*)s.c_str(), (char*)(tableOfSymbols[i].name.c_str()))) {
            return true;
        }
    }
    return false;
}

Symbol* Assembler::getSymbol(string s) {
    for(int i = 0; i < tableOfSymbols.size(); i++)
        if(!strcmp((char*)s.c_str(), (char*)(tableOfSymbols[i].name.c_str()))) 
            return &tableOfSymbols[i];
    return nullptr;
}

void Assembler::updateSymbol(Symbol s) {
    int index = -1;
    for(int i = 0; i < tableOfSymbols.size(); i++)
        if(!strcmp(s.name.c_str(), tableOfSymbols[i].name.c_str())) index = i;
    //
    if(index > -1) {
        tableOfSymbols[index].offset = s.offset;
        tableOfSymbols[index].sectionNumber = currentSection->number;
        tableOfSymbols[index].sectionName = currentSection->name;
        if(!tableOfSymbols[index].defined) tableOfSymbols[index].defined = true;
        else throw runtime_error("Can't define the same symbol twice");
    }
    else throw runtime_error("Can't find symbol in updateSymbol.");
    //fore
    for(int i = 0; i < sections.size(); i++) {
        for(int j = 0; j < sections[i].relTable.size(); j++) {     
            for(int k = 0; k < tableOfSymbols[index].forwardRefTable.size(); k++) {
                    if(sections[i].relTable[j].offset == tableOfSymbols[index].forwardRefTable[k].address && sections[i].relTable[j].id == 0 && !strcmp(sections[i].relTable[j].section.c_str(), tableOfSymbols[index].forwardRefTable[k].section->name.c_str())) sections[i].relTable[j].id = currentSection->number;
            }
        }
    }
}

void Assembler::insertSymbol(Symbol s) {
    if(s.name == s.sectionName) { // if section
        Section newSection;
        newSection.name = s.name;
        newSection.address = 0;
        newSection.size = 0;
        newSection.number = s.id;
        sections.push_back(newSection);//add new section

        if(currentSection) currentSection->size = locationCounter; // update size of old section
        currentSection = &(sections[sections.size() - 1]); //set currentSection
        locationCounter = 0;
    }
    s.id = tableOfSymbols.size();
    if(s.name == s.sectionName) {
        s.sectionNumber = s.id;
    }
    else
        s.sectionNumber = currentSection->number; // 
    tableOfSymbols.push_back(s); //add symbol
    //sort
    if (s.name == s.sectionName) { // 
        int index = -1;
        for (int i = 0; i < tableOfSymbols.size(); i++) {
            if(tableOfSymbols[i].name == tableOfSymbols[i].sectionName) continue;
            index = i;
            break;
        }
        if (index >= 0) {
            for(int i = tableOfSymbols.size() - 1; i > index; i--) {
                Symbol temp;
                temp = tableOfSymbols[i - 1];
                tableOfSymbols[i - 1] = tableOfSymbols[i];
                tableOfSymbols[i] = temp;
            }
        }
    }
}

bool Assembler::checkGlobAndExtern(string s) {
    for(int i = 0; i < globalSymbols.size(); i++) {
        if(!strcmp((char*)s.c_str(), (char*)(globalSymbols[i].c_str()))) {
            return true;
        }
    }

    for(int i = 0; i < externSymbols.size(); i++) {
        if(!strcmp((char*)s.c_str(), (char*)(externSymbols[i].c_str()))) {
            return true;
        }
    }
    
    return false;
}

bool Assembler::checkExtern(string s) {
    for(int i = 0; i < externSymbols.size(); i++) {
        if(!strcmp((char*)s.c_str(), (char*)(externSymbols[i].c_str()))) {
            return true;
        }
    }
    
    return false;
}

string Assembler::binaryToHexa(char* b, int m) {
    string binnum(b);
    char hexa[m];
    int temp;
    long int i=0,j=0; 
    while(binnum[i]) {
        binnum[i] = binnum[i] -48;// '1' i '0', 1 i 0 
        ++i;
    }
    --i;
    while(i-2>=0) {
        temp =  binnum[i-3] * 8 + binnum[i-2] * 4 +  binnum[i-1] *2 + binnum[i] ;
        if(temp > 9)
                hexa[j++] = temp + 55;
        else
                hexa[j++] = temp + 48;
        i=i-4;
    }
    if(i ==1)
            hexa[j] = binnum[i-1] *2 + binnum[i] + 48 ;
    else if(i==0)
            hexa[j] =  binnum[i] + 48 ;
    else
            --j;
    return string(hexa).substr(0,j+1);
}

string Assembler::decimalToHexa(char* b, int m) { 
    char hexaDeciNum[m];
    unsigned long n = atol(b);// bilo unsigned
    if(n == 0) {
        return "0";
    }
    if( m == 2 && (n > 255 || n < 0)) throw runtime_error("Syntax error on line " + to_string(lineCounter));//.byte argument out of scope
    //else if (m == 4 && (n > 65535 || n < 0)) throw runtime_error("Syntax error on line " + to_string(lineCounter));//.word argument out of scope
    
    // counter for hexadecimal number array 
    int i = 0; 
    while(n!=0) {    
        // temporary variable to store remainder 
        int temp  = 0; 
          
        // storing remainder in temp variable. 
        temp = n % 16; 
          
        // check if temp < 10 
        if(temp < 10) { 
            hexaDeciNum[i] = temp + 48; 
            i++; 
        } 
        else { 
            hexaDeciNum[i] = temp + 55; 
            i++; 
        } 
          
        n = n/16; 
    }
    return string(hexaDeciNum).substr(0,i); 
}

void Assembler::start(ifstream* input, ofstream* output) {
    try {
        printf("Started.\n");

        firstPass(input, output);
        
        printf("Finished.\n");
    }
    catch(exception& e) {
        cout << "Runtime error: " << e.what() <<"\n";
    }
}

bool Assembler::findEQUEntry(string s){
    for(int i = 0; i < EQUTable.size(); i++) {
        if(!strcmp(EQUTable[i].symbol.c_str(), s.c_str())) {
            return true;
        }
    }
    return false;
}

void Assembler::firstPass(ifstream* in, ofstream* out) {
    char* delimiters = (char*)" \n\t,\r\v";
    char* tkn;
    string line;
    string savedLine;
    Symbol undefined;
    sections.reserve(1000);//max number of sections
    //UND
    undefined.name = "UND";
    undefined.offset = 0;
    undefined.sectionName = "UND";
    undefined.global = false;
    undefined.id = numOfSimbols++;
    undefined.sectionNumber = 0;
    undefined.defined = true;
    tableOfSymbols.push_back(undefined);
    //ABS
    undefined.name = "ABS";
    undefined.offset = 0;
    undefined.sectionName = "ABS";
    undefined.global = false;
    undefined.id = numOfSimbols++;
    undefined.sectionNumber = 1;
    undefined.defined = true;
    tableOfSymbols.push_back(undefined);

    assemblerStatus = START;
    while(getline(*in, line)) {
        if(assemblerStatus == END_LINE || assemblerStatus == START) assemblerStatus = START_LINE;
        else throw runtime_error("Invalid assembler state.");
        savedLine = line;

       
        tkn = strtok((char*)line.c_str(), delimiters);

        if(tkn == 0) {
            assemblerStatus = END_LINE;
            lineCounter++;
            continue;
        }

        if(string(tkn).find(':') != string::npos) { //labela
            if(currentSection == 0) throw runtime_error("Not allowed to define symbol outside of sections, on line: " + to_string(lineCounter) + ".\n");
            if(!regex_match(tkn, regex("(\\w+)[:]"))) {
                throw runtime_error("Syntax error on line" + std::to_string(lineCounter) + "\n");
            }
            assemblerStatus = LABEL;
            string help = string(tkn);
            Symbol s;
            s.name = help.substr(0, help.length() - 1);
            if(checkExtern(s.name)) throw runtime_error("Cannot define symbol already labeled as extern, on line: " + to_string(lineCounter) + ".\n");
            if(findEQUEntry(tkn)) throw runtime_error("Cannot define symbol that are already defined by equ, on line: " + to_string(lineCounter) + ".\n");
            s.offset = locationCounter;

            if(findSymbol(help.substr(0, help.length() - 1)))
                updateSymbol(s);
            else {
                s.defined = true;
                s.global = checkGlobAndExtern(s.name);
                s.id = numOfSimbols++;
                s.sectionName = currentSection->name; // sectionNumber se postavlja gore u updateSymbol i insertSymbol
                insertSymbol(s);
            }

            tkn = strtok(0, delimiters);
            if(!regex_match(savedLine.c_str(), regex("(\\s*)(\\w+)[:](\\s+)[.\\s]*"))) {//ako ima nesto iza labele, podesi savedLine da ga sledeci ifovi pokupe lepo
                savedLine = savedLine.substr(savedLine.find_first_of(':')+1);
            }

            if(tkn == 0) {
                assemblerStatus = END_LINE;
                lineCounter++;
                continue;//ako je sama labela u liniji
            }  
        }

        if(!strcmp(tkn, ".end")) {// end
            break;
        }

        if(!strcmp(tkn, ".global")) { // global done
            tkn = strtok(0, delimiters);
            if(!regex_match(savedLine.c_str(), regex("(\\s*)\\.global(\\s+)(\\w+)(((\\s*)[,](\\s*)(\\w+)(\\s*))*)(\\s*)"))) {
                throw runtime_error("Syntax error on line " + std::to_string(lineCounter) + "\n");
            }
            assemblerStatus = DIRECTIVE;
            while(tkn != NULL) {
                if(findSymbol(tkn) || findEQUEntry(tkn)) throw runtime_error("Error on line " + std::to_string(lineCounter) + ", not allowed to define symbol \"" + tkn  + "\" as global after it is already used/defined.\n");
                if(!checkGlobAndExtern(tkn)) globalSymbols.push_back(tkn);
                else {
                    throw runtime_error("Error on line " + std::to_string(lineCounter) + ", symbol \"" + tkn  + "\" already defined as global/extern.\n");
                }
                tkn = strtok(0, delimiters);
            }
            assemblerStatus = END_LINE;
            lineCounter++;
            continue;
        }

        if(!strcmp(tkn, ".extern")){ //extern done
            tkn = strtok(0, delimiters);
            if(!regex_match(savedLine.c_str(), regex("(\\s*)\\.extern(\\s+)(\\w+)(((\\s*)[,](\\s*)(\\w+)(\\s*))*)(\\s*)"))) {
                throw runtime_error("Syntax error on line " + std::to_string(lineCounter) + "\n");
            }
            assemblerStatus = DIRECTIVE;

            while(tkn != NULL) {
                if(findSymbol(tkn) || findEQUEntry(tkn)) throw runtime_error("Error on line " + std::to_string(lineCounter) + ", not allowed to define symbol \"" + tkn  + "\" as extern after it is already used/defined.\n");
                if(!checkGlobAndExtern(tkn)) {
                    Symbol s;
                    s.name = string(tkn);
                    s.id = numOfSimbols++;
                    s.sectionName = "UND";
                    s.offset = 0;
                    s.global = true;
                    s.defined = false;
                    s.sectionNumber = 0;
                    tableOfSymbols.push_back(s);
                    externSymbols.push_back(tkn);
                }
                else {
                    throw runtime_error("Error on line " + std::to_string(lineCounter) + ", symbol \"" + tkn  + "\" already defined as global/extern.\n");
                }
                tkn = strtok(0, delimiters);
            }
            //
            assemblerStatus = END_LINE;
            lineCounter++;
            continue;
        }

        if(!strcmp(tkn, ".section")){ //sections done
            tkn = strtok(0, " \t");
            if(!regex_match(savedLine.c_str(), regex("(\\s*)\\.section(\\s+)(\\w+)(\\s*)"))) {
                throw runtime_error("Syntax error on line " + std::to_string(lineCounter) + "\n");
            }
            assemblerStatus = SECTION;
            //greske
            if(findSymbol(tkn) || findEQUEntry(tkn)) throw runtime_error("Error on line " + to_string(lineCounter) + ", symbol already used/defined.\n");// nisam dozvolio updateSymbol za sekciju, odnosno nema forwardRef zapisa za sekciju
            if(checkGlobAndExtern(tkn)) throw runtime_error("Error on line " + to_string(lineCounter) + ", symbol with same name already defined as global/extern.\n");
            //dodaj sekciju
            Symbol newSymbol;
            newSymbol.name = tkn;
            newSymbol.offset = 0;
            newSymbol.sectionName = tkn;
            newSymbol.defined = true;
            newSymbol.global = false;
            newSymbol.id = numOfSimbols++;
            insertSymbol(newSymbol);
            //
            lineCounter++;
            assemblerStatus = END_LINE;
            continue;
        }

        if(!strcmp(tkn, ".skip")) { // skip done
            if(currentSection == 0) throw runtime_error("Error on line " + to_string(lineCounter) + ", not allowed to write directives/commands outside of sections.\n");
            tkn = strtok(0, " \t");
            if(!regex_match(savedLine.c_str(), regex("(\\s*)\\.skip(\\s+)(\\d+)(\\s*)")) && !regex_match(savedLine.c_str(), regex("(\\s*)\\.skip(\\s+)0[xX][a-fA-F0-9]+(\\s*)")) && !regex_match(savedLine.c_str(), regex("(\\s*)\\.skip(\\s+)0[bB][0-1]+(\\s*)"))) {
                throw runtime_error("Error on line "+ to_string(lineCounter) + ", invalid argument for .skip .\n");
            }
            assemblerStatus = DIRECTIVE;

            string help(tkn);
            if(help.find('x') != string::npos || help.find("X") != string::npos) { // hexa
                help = help.substr(2);
                char* num = (char*)help.c_str();
                int len = strlen(num);
                int base = 1;
                int temp = 0;
                for (int i=len-1; i>=0; i--) {
                    if (num[i]>='0' && num[i]<='9') {
                        temp += (num[i] - 48)*base;
                        base = base * 16;
                    }
                    else if (num[i]>='A' && num[i]<='F') {
                        temp += (num[i] - 55)*base;
                        base = base*16;
                    }
                    else if (num[i]>='a' && num[i]<='f') {
                        temp += (num[i] - 87)*base;
                        base = base*16;
                    }
                }
                locationCounter += temp;
                help = "";
                for(int i = 0; i < temp; i++) help+="00 ";
                currentSection->content += help;
            }
            else if(help.find('b') != string::npos || help.find("B") != string::npos) { //binary
                help = help.substr(2);
                long long num = atoi(help.c_str());
                int decimalNum, i, rem;
                decimalNum = 0;
                i = 0;
                while (num != 0) {
                    rem = num % 10;
                    num /= 10;
                    decimalNum += rem * pow(2, i);
                    ++i;
                }
                locationCounter += decimalNum;
                help = "";
                for(int i = 0; i < decimalNum; i++) help+="00 ";
                currentSection->content += help;
            }
            else {
                locationCounter += atoi(tkn);
                help = "";
                for(int i = 0; i < atoi(tkn); i++) help+="00 ";
                currentSection->content += help;
            }
            //
            assemblerStatus = END_LINE;
            lineCounter++;
            continue;
        }

        if(!strcmp(tkn, ".byte")){ // byte done
            if(currentSection == 0) throw runtime_error("Error on line " + to_string(lineCounter) + ", not allowed to write directives/commands outside of sections.\n");
            assemblerStatus = DIRECTIVE;

            tkn = strtok(0, delimiters);
            if(!regex_match(savedLine.c_str(), regex("(\\s*)\\.byte(\\s+)((\\w+)|(\\d+)|(0[bB][0-1]+)|(0[xX][a-fA-F0-9]+))(((\\s*)[,](\\s*)((\\w+)|(\\d+)|(0[bB][0-1]+)|(0[xX][a-fA-F0-9]+))(\\s*))*)(\\s*)"))) {
                throw runtime_error("Syntax error on line " + std::to_string(lineCounter) + "\n");
            }
            //
            while(tkn != NULL) {//0x0000000001
                if(*tkn == '0' && (*(tkn+1)=='x' || *(tkn+1)=='X')) { //hex number
                    if(!regex_match(tkn, regex("0[xX][0]*[a-fA-F0-9]{1,2}")))  throw runtime_error("Syntax error, operand too big, on line " + std::to_string(lineCounter) + "\n");
                    string help(tkn);
                    //conver to upper
                    if(help[help.length()-1] <= 122 && help[help.length()-1] >= 97) help[help.length()-1] -= 32;
                    if(help[help.length()-2] <= 122 && help[help.length()-2] >= 97) help[help.length()-2] -= 32;
                    //add content
                    help = help.substr(2);
                    if(help.length() == 1) {
                        currentSection->content += "0";
                    }
                    else {
                        currentSection->content += help[help.length()-2];
                    }
                    currentSection->content += help[help.length()-1];
                    currentSection->content += " ";
                }
                else if(*tkn == '0' && (*(tkn+1)=='b' || *(tkn+1)=='B')) { //bin number
                    if(!regex_match(tkn, regex("0[bB][0]*[0-1]{1,8}"))) throw runtime_error("Syntax error, operand too big, on line " + std::to_string(lineCounter) + "\n");
                    //to hexa, pojedi prve nule
                    int index = 0;
                    tkn+=2;
                    index = string(tkn).find_first_not_of('0');
                    if(index == string::npos) index = 0;
                    string help = binaryToHexa(tkn+index, 2);
                    //add content
                    if(help.length() == 1) {
                        currentSection->content += "0";
                        currentSection->content += help[help.length()-1];
                    }
                    else {
                        currentSection->content += help[help.length()-1];
                        currentSection->content += help[help.length()-2];
                    }
                    currentSection->content += " ";
                }
                else if (regex_match(tkn, regex("(\\d+)"))) { // dec
                    string help = decimalToHexa(tkn, 2);
                    if(atoi(tkn) > 255) throw runtime_error("Syntax error, operand too big, on line " + std::to_string(lineCounter) + "\n");
                    if(help.length() == 1) {
                        currentSection->content += "0";
                        currentSection->content += help[help.length()-1];
                    }
                    else {
                        currentSection->content += help[help.length()-1];
                        currentSection->content += help[help.length()-2];
                    }
                    currentSection->content += " ";
                }
                else {
                    Symbol s;
                    s.name = string(tkn);
                    if(findSymbol(s.name)) {
                        Symbol* sPtr =getSymbol(s.name);

                        if(sPtr->global){//globalni simbol ili eksterni simbol
                            Relocation newRel;
                            newRel.id = sPtr->id;
                            newRel.offset = locationCounter;
                            newRel.section = currentSection->name;
                            newRel.type = "32";
                            //vrednost simbola je 00
                            currentSection->content += "00 ";
                            currentSection->relTable.push_back(newRel);
                        }
                        else {//lokalni simbol
                            Relocation newRel;
                            newRel.id = 0;
                            newRel.offset = locationCounter;
                            newRel.section = currentSection->name;
                            newRel.type = "32";

                            if(sPtr->defined) {
                                newRel.id = sPtr->sectionNumber;
                                //upisi vrednost
                                string help = decimalToHexa((char*)to_string(sPtr->offset).c_str(),2);
                                if(sPtr->offset > 255) throw runtime_error("Syntax error, operand too big, on line " + std::to_string(lineCounter) + "\n");
                                if(help.length() == 1) {
                                    currentSection->content += "0";
                                    currentSection->content += help[help.length()-1];
                                    currentSection->content += " ";
                                }
                                else {
                                    currentSection->content += help[help.length()-1];
                                    currentSection->content += help[help.length()-2];
                                    currentSection->content += " ";
                                }
                            }
                            else {
                                forwardRefEntry f;
                                f.address = locationCounter;
                                f.byte = true;
                                f.section = currentSection;
                                sPtr->forwardRefTable.push_back(f);
                                currentSection->content += "00 ";
                            }
                            currentSection->relTable.push_back(newRel);
                        }
                    } 
                    else {
                        s.defined = false;
                        s.sectionName = "";
                        s.sectionNumber = 0;
                        s.offset = 0;
                        s.id = numOfSimbols++;
                        Relocation newRel;
                        newRel.offset = locationCounter;
                        newRel.section = currentSection->name;
                        newRel.type = "32";

                        if(checkGlobAndExtern(s.name)) { // da li je global, ovo prolazi samo ako je global jer da je ekstern bio bi vec u tabeli
                            s.global = true;
                            //napravi rel zapis
                            newRel.id = s.id;
                        }
                        else {// lokalan
                            s.global = false;
                            //napravi rel zapis
                            newRel.id = 0;// ne zna se sekcija lokalnog simbola pa je u onim gore forama postavljamo
                            forwardRefEntry f;
                            f.address = locationCounter;
                            f.byte = true;
                            f.section = currentSection;
                            s.forwardRefTable.push_back(f);
                        }

                        currentSection->relTable.push_back(newRel);
                        tableOfSymbols.push_back(s);
                        currentSection->content += "00 ";
                    }
                }
                locationCounter++;
                tkn = strtok(0, delimiters);
            }
            //
            assemblerStatus = END_LINE;
            lineCounter++;
            continue;
        }

        if(!strcmp(tkn, ".word")) {//word done
            if(currentSection == 0) throw runtime_error("Error on line " + to_string(lineCounter) + ", not allowed to write directives/commands outside of sections.\n");
            tkn = strtok(0, delimiters);
            if(!regex_match(savedLine.c_str(), regex("(\\s*)\\.word(\\s+)((\\w+)|(\\d+)|(0[bB][0-1]+)|(0[xX][a-fA-F0-9]+))(((\\s*)[,](\\s*)((\\w+)|(\\d+)|(0[bB][0-1]+)|(0[xX][a-fA-F0-9]+))(\\s*))*)(\\s*)"))) {
                throw runtime_error("Syntax error on line " + std::to_string(lineCounter) + "\n");
            }
            assemblerStatus = DIRECTIVE;
            //
            while(tkn != NULL) { //
                if(*tkn == '0' && (*(tkn+1)=='x' || *(tkn+1)=='X')) { //hex number
                    if(!regex_match(tkn, regex("0[xX][0]*[a-fA-F0-9]{1,4}")))  throw runtime_error("Syntax error, operand too big, on line " + std::to_string(lineCounter) + "\n");
                    string help(tkn);
                    help = help.substr(2);
                    if(help.find_first_not_of('0') != string::npos) help = help.substr(help.find_first_not_of('0'));
                    else help = "0";
                    //conver to upper
                    if(help[help.length()-1] <= 122 && help[help.length()-1] >= 97) help[help.length()-1] -= 32;
                    if(help[help.length()-2] <= 122 && help[help.length()-2] >= 97) help[help.length()-2] -= 32;
                    if(help[help.length()-3] <= 122 && help[help.length()-3] >= 97) help[help.length()-3] -= 32;
                    if(help[help.length()-4] <= 122 && help[help.length()-4] >= 97) help[help.length()-4] -= 32;
                    //add content
                    switch (help.length()) {//
                        case 4:
                            currentSection->content += help[help.length()-2];
                            currentSection->content += help[help.length()-1];
                            currentSection->content += " ";
                            currentSection->content += help[help.length()-4];
                            currentSection->content += help[help.length()-3];
                            currentSection->content += " ";
                            break;
                        case 3:
                            currentSection->content += help[help.length()-2];
                            currentSection->content += help[help.length()-1];
                            currentSection->content += " 0";
                            currentSection->content += help[help.length()-3];
                            currentSection->content += " ";
                            break;
                        case 2:
                            currentSection->content += help[help.length()-2];
                            currentSection->content += help[help.length()-1];
                            currentSection->content += " 00 ";
                            break;
                        case 1:
                            currentSection->content += "0";
                            currentSection->content += help[help.length()-1];
                            currentSection->content += " 00 ";
                            break;
                        default:
                            throw runtime_error("Default branch in switch case in .word .");
                            break;
                    }
                }
                else if(*tkn == '0' && (*(tkn+1)=='b' || *(tkn+1)=='B')) { //bin number
                    if(!regex_match(tkn, regex("0[bB][0]*[0-1]{1,16}"))) throw runtime_error("Syntax error, operand too big, on line " + std::to_string(lineCounter) + "\n");
                    //to hexa, pojedi prve nule
                    int index = 0;
                    tkn+=2;
                    index = string(tkn).find_first_not_of('0');
                    string help ;
                    if(index == string::npos) help = "0";
                    else help = binaryToHexa(tkn+index, 4);
                    //add content
                    switch (help.length()) {//
                        case 4:
                            currentSection->content += help[help.length()-3];
                            currentSection->content += help[help.length()-4];
                            currentSection->content += " ";
                            currentSection->content += help[help.length()-1];
                            currentSection->content += help[help.length()-2];
                            currentSection->content += " ";
                            break;
                        case 3:
                            currentSection->content += help[1];
                            currentSection->content += help[0];
                            currentSection->content += " 0";
                            currentSection->content += help[2];
                            currentSection->content += " ";
                            break;
                        case 2:
                            currentSection->content += help[help.length()-1];
                            currentSection->content += help[help.length()-2];
                            currentSection->content += " 00 ";
                            break;
                        case 1:
                            currentSection->content += "0";
                            currentSection->content += help[help.length()-1];
                            currentSection->content += " 00 ";
                            break;
                        default:
                            throw runtime_error("Default branch in switch case in .word .");
                            break;
                    }
                }
                else if (regex_match(tkn, regex("(\\d+)"))) { // dec
                    if(atoi(tkn) > 65535) throw runtime_error("Syntax error, operand too big, on line " + std::to_string(lineCounter) + "\n");
                    string help = decimalToHexa(tkn, 4);
                    switch (help.length()) {// 03 04 --- 0403
                        case 4:
                            currentSection->content += help[help.length()-3];
                            currentSection->content += help[help.length()-4];
                            currentSection->content += " ";
                            currentSection->content += help[help.length()-1];
                            currentSection->content += help[help.length()-2];
                            currentSection->content += " ";
                            break;
                        case 3:
                            currentSection->content += help[1];
                            currentSection->content += help[0];
                            currentSection->content += " 0";
                            currentSection->content += help[2];
                            currentSection->content += " ";
                            break;
                        case 2:
                            currentSection->content += help[help.length()-1];
                            currentSection->content += help[help.length()-2];
                            currentSection->content += " 00 ";
                            break;
                        case 1:
                            currentSection->content += "0";
                            currentSection->content += help[help.length()-1];
                            currentSection->content += " 00 ";
                            break;
                        default:
                            throw runtime_error("Default branch in switch case in .word .");
                            break;
                    }
                }
                else { // symbol, menjaj za rel tabele
                    Symbol s;
                    s.name = string(tkn);
                    if(findSymbol(s.name)) {
                        Symbol* sPtr =getSymbol(s.name);

                        if(sPtr->global){//globalni simbol ili eksterni simbol
                            Relocation newRel;
                            newRel.id = sPtr->id;
                            newRel.offset = locationCounter;
                            newRel.section = currentSection->name;
                            newRel.type = "32";
                            //vrednost simbola je 00
                            currentSection->content += "00 00 ";
                            currentSection->relTable.push_back(newRel);
                        }
                        else {//lokalni simbol
                            Relocation newRel;
                            newRel.id = 0;
                            newRel.offset = locationCounter;
                            newRel.section = currentSection->name;
                            newRel.type = "32";

                            if(sPtr->defined) {
                                //upisi vrednost
                                newRel.id = sPtr->sectionNumber;
                                string help = decimalToHexa((char*)to_string(sPtr->offset).c_str(),4);
                                if(sPtr->offset > 65535) throw runtime_error("Syntax error, operand too big, on line " + std::to_string(lineCounter) + "\n");
                                switch (help.length()) {// 03 04 --- 0403
                                case 4:
                                    currentSection->content += help[help.length()-3];
                                    currentSection->content += help[help.length()-4];
                                    currentSection->content += " ";
                                    currentSection->content += help[help.length()-1];
                                    currentSection->content += help[help.length()-2];
                                    currentSection->content += " ";
                                    break;
                                case 3:
                                    currentSection->content += help[1];
                                    currentSection->content += help[0];
                                    currentSection->content += " 0";
                                    currentSection->content += help[2];
                                    currentSection->content += " ";
                                    break;
                                case 2:
                                    currentSection->content += help[help.length()-1];
                                    currentSection->content += help[help.length()-2];
                                    currentSection->content += " 00 ";
                                    break;
                                case 1:
                                    currentSection->content += "0";
                                    currentSection->content += help[help.length()-1];
                                    currentSection->content += " 00 ";
                                    break;
                                default:
                                    throw runtime_error("Default branch in switch case in .word .");
                                    break;
                                }
                            }
                            else {
                                forwardRefEntry f;
                                f.address = locationCounter;
                                f.byte = false;
                                f.section = currentSection;
                                sPtr->forwardRefTable.push_back(f);
                                currentSection->content += "00 00 ";
                            }
                            currentSection->relTable.push_back(newRel);
                        }
                    } 
                    else {
                        s.defined = false;
                        s.sectionName = "";
                        s.sectionNumber = 0;
                        s.offset = 0;
                        s.id = numOfSimbols++;
                        Relocation newRel;
                        newRel.offset = locationCounter;
                        newRel.section = currentSection->name;
                        newRel.type = "32";

                        if(checkGlobAndExtern(s.name)) { // da li je global, ovo prolazi samo ako je global jer da je ekstern bio bi vec u tabeli
                            s.global = true;
                            //napravi rel zapis
                            newRel.id = s.id;
                        }
                        else {// lokalan
                            s.global = false;
                            //napravi rel zapis
                            newRel.id = 0;// ne zna se sekcija lokalnog simbola pa je u onim gore forama postavljamo
                            forwardRefEntry f;
                            f.address = locationCounter;
                            f.byte = false;
                            f.section = currentSection;
                            s.forwardRefTable.push_back(f);
                        }

                        currentSection->relTable.push_back(newRel);
                        tableOfSymbols.push_back(s);
                        currentSection->content += "00 00 ";
                    }
                }
                locationCounter += 2;
                tkn = strtok(0, delimiters);
            }
            //
            assemblerStatus = END_LINE;
            lineCounter++;
            continue;
        }
        
        if(!strcmp(tkn, ".equ")) {//equ done
            if(!regex_match(savedLine.c_str(), regex("^(\\s*)\\.equ(\\s+)(\\w+)(\\s*)[,](\\s*)[\\+\\-]{0,1}(\\s*)((\\w+)|(\\d+)|(0[bB][0-1]+)|(0[xX][a-fA-F0-9]+))((\\s*)[\\+\\-](\\s*)((\\w+)|((\\d+)|(0[bB][0-1]+)|(0[xX][a-fA-F0-9]+)))(\\s*))*(\\s*)$"))) {
                throw runtime_error("Syntax error on line " + std::to_string(lineCounter) + "\n");
            }
            //equ logika
            tkn = strtok(0, delimiters);
            string equSymbol(tkn);
            //da li je vec def simbol
            if(findEQUEntry(equSymbol) || (getSymbol(equSymbol) && checkExtern(equSymbol)) || (getSymbol(equSymbol) && getSymbol(equSymbol)->defined)) throw runtime_error("Error on line " + std::to_string(lineCounter) + ", symbol already defined.\n");
            // + ili -
            stringstream ss;
            bool negative = false;
            char signChar;
            //
            ss.str(savedLine.substr(savedLine.find_first_of(',') + 1));
            ss >> signChar;
            //dole ifovi pokrivaju situaciju kada je izraz: +/- simbol1 + simbol2..., zbog prvog +/-
            if(signChar == '-') {// ako je prvi - onda postavi neg
                negative = true;
            }
            else if(signChar != '-' && signChar != '+') {//ako nije ni + ni - prvi char onda si pokupio deo nekog simbola i posto je editovan ss nakon >>, moramo ga resetovati
                ss.str(savedLine.substr(savedLine.find_first_of(',') + 1));
            }
            //ubaci equ
            EQUTableRow row;
            row.symbol = equSymbol;
            //
            //bool lastToken = false;
            if(ss.str().substr(ss.tellg()).find_first_of('+') > ss.str().substr(ss.tellg()).find_first_of('-')) {
                ss.get(tkn, 100, '-');
            }//pre ima minus
            else if(ss.str().substr(ss.tellg()).find_first_of('+') < ss.str().substr(ss.tellg()).find_first_of('-')) {
                ss.get(tkn, 100, '+');  
            }//pre ima plus
            else {//nema vise znakova
                ss.get(tkn, 100);
            }
            while(1) {
                //
                string tknString(tkn);
                tknString = tknString.substr(tknString.find_first_not_of(' '), tknString.find_last_not_of(' ') + 1 - tknString.find_first_not_of(' '));//pojede spaceove
                strcpy(tkn, tknString.c_str());
                //tkn = (char*)tknString.c_str();
                //
                //prolazimo kroz izraz
                EQUPair pair;
                pair.symbolName = tkn;
                if(regex_match(tkn, regex("(([0-9]+)|(0[bB][01]+)|(0[xX][a-fA-F0-9]+))"))) {//brojevi
                    pair.symbolName = "";
                    pair.section = "";
                    if(regex_match(tkn, regex("^(\\d+)$"))) { // decimal
                        pair.value = atol(tkn);
                        if(negative) pair.value = - pair.value;
                    }
                    else if(regex_match(tkn, regex("^(0[xX][0-9a-fA-F]+)$"))) { // hexa
                        pair.value = strtol(tkn+2,NULL,16);
                        if(negative) pair.value = - pair.value;

                    }
                    else { // bin
                        pair.value = strtol(tkn+2, NULL, 2);
                        if(negative) pair.value = - pair.value;
                    }
                }
                else if(checkExtern(tkn)) {//ekstern
                    pair.value = 1;
                    pair.section = "UND";
                }
                else {
                    pair.value = (negative ? -1 : 1);
                    if(getSymbol(tkn) && getSymbol(tkn)->defined) {//definisan simbol
                        pair.section = getSymbol(tkn)->sectionName;
                        if(!strcmp(getSymbol(tkn)->sectionName.c_str(), "ABS")) pair.value *= getSymbol(tkn)->offset;//ako je ABS simbol samo ubaci njegovu vrednost kao da je literal
                    }
                    else {//nema ga ili nije definisan
                        pair.section = "";
                        if(!getSymbol(tkn)) {
                            Symbol s;
                            s.name = tkn;
                            if(checkGlobAndExtern(tkn)) s.global = true;
                            else s.global = false;
                            s.defined = false;
                            s.id = numOfSimbols++;
                            s.sectionName = "";
                            tableOfSymbols.push_back(s);
                        }
                    }
                    //
                }
                //
                row.table.push_back(pair);
                //next
                negative = false;
                //
                //if(lastToken) break;
                //
                ss >> signChar;
                if(signChar == '-') {// ako je prvi - onda postavi neg
                    negative = true;
                }
                if((int)ss.tellg() == -1) break;
                if(ss.str().substr(ss.tellg()).find_first_of('+') > ss.str().substr(ss.tellg()).find_first_of('-')) {
                    ss.get(tkn, 100, '-');
                }//pre ima minus
                else if(ss.str().substr(ss.tellg()).find_first_of('+') < ss.str().substr(ss.tellg()).find_first_of('-')) {
                    ss.get(tkn, 100, '+');  
                }//pre ima plus
                else {//nema vise znakova
                    ss.get(tkn, 100);
                }
                /*ss >> tknString;
                if(ss.eof()) {
                    printf("KRAJ %s, %s\n", tknString.c_str(), ss.str().c_str());
                    lastToken = true;
                    break;
                };//namesti za sled krug da ti vrati ss >> tknString prazan string u tknString
                //
                strcpy(tkn, tknString.c_str());*/
            }
            //da li moze odma da se izracuna
            vector<string> sections = vector<string>();
            vector<int> values = vector<int>();
            bool pushedInEQUTable = false;
            long literalValues = 0;
            for(int i = 0; i < row.table.size(); i++) {
                //
                if(!strcmp(row.table[i].section.c_str(), "") && strcmp(row.table[i].symbolName.c_str(), "")) {
                    EQUTable.push_back(row);
                    pushedInEQUTable = true;
                    break;
                }
               //
                if((!strcmp(row.table[i].section.c_str(), "") && !strcmp(row.table[i].symbolName.c_str(), "")) || !strcmp(row.table[i].section.c_str(), "ABS")) {//ako je literal
                    literalValues+=row.table[i].value;
                    continue;
                }
                int index = -1;
                for(int j = 0; j < sections.size(); j++) {
                    if(!strcmp(sections[j].c_str(), row.table[i].section.c_str())) {
                        index = j;
                    }
                }
                if(index != -1) {
                    values[index] += row.table[i].value;
                }
                else {
                    sections.push_back(row.table[i].section);
                    values.push_back(row.table[i].value);
                }
            }
            if(!pushedInEQUTable) {
                int foundOne = -1;
                for(int i = 0; i < sections.size(); i++) {
                    if((foundOne != -1 && values[i] != 0) || (values[i] != 0 && values[i] != 1)) throw runtime_error("EQU statement for symbol \"" + equSymbol + "\" not valid.\n");
                    if(values[i] == 1) foundOne = i;
                }
                Symbol toInsert;
                toInsert.defined = true;
                toInsert.id = numOfSimbols++;
                toInsert.name = equSymbol;
                toInsert.offset = literalValues;
                toInsert.global = false;
                if(checkGlobAndExtern(equSymbol)) toInsert.global = true;
                if(foundOne == -1) {
                    toInsert.sectionName = "ABS";
                    toInsert.sectionNumber = 1;
                }
                else {
                    toInsert.sectionName = sections[foundOne];
                    toInsert.sectionNumber = getSymbol(toInsert.sectionName)->id;
                }
                //racunanje toInsert.offset
                for(int i = 0; i < row.table.size(); i++) {
                    if(strcmp(row.table[i].symbolName.c_str(), "")) {
                       if(row.table[i].value == 1) toInsert.offset += getSymbol(row.table[i].symbolName)->offset;
                       else toInsert.offset -= getSymbol(row.table[i].symbolName)->offset;
                    }
                }
                //ubaci simbol u tabelu
                if(getSymbol(toInsert.name)) {
                    updateSymbol(toInsert);
                    getSymbol(toInsert.name)->sectionName = toInsert.sectionName;//u update symbol se uvek postavlja sectionName na currSection->name, pa ko zna sta bi se desilo kad bi to menjao pa cu ovako
                    getSymbol(toInsert.name)->sectionNumber = toInsert.sectionNumber;
                }
                else  tableOfSymbols.push_back(toInsert);
            }
            //
            lineCounter++;
            assemblerStatus = END_LINE;
            continue;
        }

        // asemblerske naredbe
        if(!strcmp(tkn, "halt")) { // halt
            if(currentSection == 0) throw runtime_error("Error on line " + to_string(lineCounter) + ", not allowed to write directives/commands outside of sections.\n");
            if(!regex_match(savedLine.c_str(), regex("(\\s*)halt(\\s*)"))) {
                throw runtime_error("Syntax error on line " + std::to_string(lineCounter) + "\n");
            }
            assemblerStatus = INSTRUCTION;

            string help =  decimalToHexa((char*)to_string(HALT).c_str(), 2);
            if(help.length() == 1) {
                currentSection->content += "0";
                currentSection->content += help[help.length()-1];
            }
            else {
                currentSection->content += help[help.length()-1];
                currentSection->content += help[help.length()-2];
            }
            currentSection->content += " ";
            locationCounter++;

            assemblerStatus = END_LINE;
            lineCounter++;
            continue;
        }

        if(!strcmp(tkn, "iret")) { // iret
            if(currentSection == 0) throw runtime_error("Error on line " + to_string(lineCounter) + ", not allowed to write directives/commands outside of sections.\n");
            if(!regex_match(savedLine.c_str(), regex("(\\s*)iret(\\s*)"))) {
                throw runtime_error("Syntax error on line " + std::to_string(lineCounter) + "\n");
            }
            assemblerStatus = INSTRUCTION;

            string help =  decimalToHexa((char*)to_string(IRET * 8).c_str(), 2);
            if(help.length() == 1) {
                currentSection->content += "0";
                currentSection->content += help[help.length()-1];
            }
            else {
                currentSection->content += help[help.length()-1];
                currentSection->content += help[help.length()-2];
            }
            currentSection->content += " ";
            locationCounter++;

            assemblerStatus = END_LINE;
            lineCounter++;
            continue;
        }

        if(!strcmp(tkn, "ret")) { // ret
            if(currentSection == 0) throw runtime_error("Error on line " + to_string(lineCounter) + ", not allowed to write directives/commands outside of sections.\n");
            if(!regex_match(savedLine.c_str(), regex("(\\s*)ret(\\s*)"))) {
                throw runtime_error("Syntax error on line " + std::to_string(lineCounter) + "\n");
            }
            assemblerStatus = INSTRUCTION;

            string help =  decimalToHexa((char*)to_string(RET * 8).c_str(), 2);
            if(help.length() == 1) {
                currentSection->content += "0";
                currentSection->content += help[help.length()-1];
            }
            else {
                currentSection->content += help[help.length()-1];
                currentSection->content += help[help.length()-2];
            }
            currentSection->content += " ";
            locationCounter++;

            assemblerStatus = END_LINE;
            lineCounter++;
            continue;
        }
        
        //skini w/b poslednji char
        char sizeOfOperand = 'w';
        string helpForSizeOfOP(tkn);
        if(!strcmp(tkn, "subb") || (strcmp(tkn, "sub") && (helpForSizeOfOP[helpForSizeOfOP.size() - 1] == 'b' || helpForSizeOfOP[helpForSizeOfOP.size() - 1] == 'w') )) {
            if(helpForSizeOfOP[helpForSizeOfOP.size() - 1] == 'b') sizeOfOperand = 'b';
            helpForSizeOfOP.pop_back();
        }
        tkn = (char*)helpForSizeOfOP.c_str();
        //big if
        if(!strcmp(tkn, "int") || !strcmp(tkn, "call") || !strcmp(tkn, "jmp") || !strcmp(tkn, "jeq") ||
            !strcmp(tkn, "jne") || !strcmp(tkn, "jgt") || !strcmp(tkn, "push") || !strcmp(tkn, "pop") ||
            !strcmp(tkn, "xchg") || !strcmp(tkn, "mov") || !strcmp(tkn, "add") || !strcmp(tkn, "sub") ||
            !strcmp(tkn, "mul") || !strcmp(tkn, "div") || !strcmp(tkn, "cmp") || !strcmp(tkn, "not") ||
            !strcmp(tkn, "and") || !strcmp(tkn, "or") || !strcmp(tkn, "xor") || !strcmp(tkn, "test") ||
            !strcmp(tkn, "shl") || !strcmp(tkn, "shr")) {
            if(currentSection == 0) throw runtime_error("Error on line " + to_string(lineCounter) + ", not allowed to write directives/commands outside of sections.\n");
            locationCounter++;//1 bajt za instruction

            assemblerStatus = INSTRUCTION;

            bool isJump = false;
            bool singleOperand = false;
            bool dstNoImmed = false;
            unsigned long instruction = 0;
            if(!strcmp(tkn, "int")) {
                if(!regex_match(savedLine.c_str(), regex("(\\s*)int[ bw](\\s*)[^,]+(\\s*)"))) {
                    throw runtime_error("Syntax error on line " + std::to_string(lineCounter) + "\n");
                }
                isJump = true;
                singleOperand = true;
                instruction = INT * 8;
                if(sizeOfOperand == 'w') instruction += 4;
            }
            else if(!strcmp(tkn, "call")) {
                if(!regex_match(savedLine.c_str(), regex("(\\s*)call[ bw](\\s*)[^,]+(\\s*)"))) {
                    throw runtime_error("Syntax error on line " + std::to_string(lineCounter) + "\n");
                }
                isJump = true;
                singleOperand = true;
                instruction = CALL * 8;
                if(sizeOfOperand == 'w') instruction += 4;
            }
            else if(!strcmp(tkn, "jmp")) {
                if(!regex_match(savedLine.c_str(), regex("(\\s*)jmp[ bw](\\s*)[^,]+(\\s*)"))) {
                    throw runtime_error("Syntax error on line " + std::to_string(lineCounter) + "\n");
                }
                isJump = true;
                singleOperand = true;
                instruction = JMP * 8;
                if(sizeOfOperand == 'w') instruction += 4;
            }
            else if(!strcmp(tkn, "jeq")) {
                if(!regex_match(savedLine.c_str(), regex("(\\s*)jeq[ bw](\\s*)[^,]+(\\s*)"))) {
                    throw runtime_error("Syntax error on line " + std::to_string(lineCounter) + "\n");
                }
                isJump = true;
                singleOperand = true;
                instruction = JEQ * 8;
                if(sizeOfOperand == 'w') instruction += 4;
            }
            else if(!strcmp(tkn, "jne")) {
                if(!regex_match(savedLine.c_str(), regex("(\\s*)jne[ bw](\\s*)[^,]+(\\s*)"))) {
                    throw runtime_error("Syntax error on line " + std::to_string(lineCounter) + "\n");
                }
                isJump = true;
                singleOperand = true;
                instruction = JNE * 8;
                if(sizeOfOperand == 'w') instruction += 4;
            }
            else if(!strcmp(tkn, "jgt")) {
                if(!regex_match(savedLine.c_str(), regex("(\\s*)jgt[ bw](\\s*)[^,]+(\\s*)"))) {
                    throw runtime_error("Syntax error on line " + std::to_string(lineCounter) + "\n");
                }
                isJump = true;
                singleOperand = true;
                instruction = JGT * 8;
                if(sizeOfOperand == 'w') instruction += 4;
            }
            else if(!strcmp(tkn, "push")) {
                if(!regex_match(savedLine.c_str(), regex("(\\s*)push[ bw](\\s*)[^,]+(\\s*)"))) {
                    throw runtime_error("Syntax error on line " + std::to_string(lineCounter) + "\n");
                }
                dstNoImmed = true;
                singleOperand = true;
                instruction = PUSH * 8;
                if(sizeOfOperand == 'w') instruction += 4;
            }
            else if(!strcmp(tkn, "pop")) {
                if(!regex_match(savedLine.c_str(), regex("(\\s*)pop[ bw](\\s*)[^,]+(\\s*)"))) {
                    throw runtime_error("Syntax error on line " + std::to_string(lineCounter) + "\n");
                }
                dstNoImmed = true;
                singleOperand = true;
                instruction = POP * 8;
                if(sizeOfOperand == 'w') instruction += 4;
            }
            else if(!strcmp(tkn, "xchg")) {
                if(!regex_match(savedLine.c_str(), regex("(\\s*)xchg[ bw](\\s*)[^,]+(\\s*)[,](\\s*)[^,]+(\\s*)"))) {
                    throw runtime_error("Syntax error on line " + std::to_string(lineCounter) + "\n");
                }
                dstNoImmed = true;
                instruction = XCHG * 8;
                if(sizeOfOperand == 'w') instruction += 4;
            }
            else if(!strcmp(tkn, "mov")) {
                if(!regex_match(savedLine.c_str(), regex("(\\s*)mov[ bw](\\s*)[^,]+(\\s*)[,](\\s*)[^,]+(\\s*)"))) {
                    throw runtime_error("Syntax error on line " + std::to_string(lineCounter) + "\n");
                }
                dstNoImmed = true;
                instruction = MOV * 8;
                if(sizeOfOperand == 'w') instruction += 4;
            }
            else if(!strcmp(tkn, "add")) {
                if(!regex_match(savedLine.c_str(), regex("(\\s*)add[ bw](\\s*)[^,]+(\\s*)[,](\\s*)[^,]+(\\s*)"))) {
                    throw runtime_error("Syntax error on line " + std::to_string(lineCounter) + "\n");
                }
                dstNoImmed = true;
                instruction = ADD * 8;
                if(sizeOfOperand == 'w') instruction += 4;
            }
            else if(!strcmp(tkn, "sub")) {
                if(!regex_match(savedLine.c_str(), regex("(\\s*)sub[ bw](\\s*)[^,]+(\\s*)[,](\\s*)[^,]+(\\s*)"))) {
                    throw runtime_error("Syntax error on line " + std::to_string(lineCounter) + "\n");
                }
                dstNoImmed = true;
                instruction = SUB * 8;
                if(sizeOfOperand == 'w') instruction += 4;
            }
            else if(!strcmp(tkn, "mul")) {
                if(!regex_match(savedLine.c_str(), regex("(\\s*)mul[ bw](\\s*)[^,]+(\\s*)[,](\\s*)[^,]+(\\s*)"))) {
                    throw runtime_error("Syntax error on line " + std::to_string(lineCounter) + "\n");
                }
                dstNoImmed = true;
                instruction = MUL * 8;
                if(sizeOfOperand == 'w') instruction += 4;
            }
            else if(!strcmp(tkn, "div")) {
                if(!regex_match(savedLine.c_str(), regex("(\\s*)div[ bw](\\s*)[^,]+(\\s*)[,](\\s*)[^,]+(\\s*)"))) {
                    throw runtime_error("Syntax error on line " + std::to_string(lineCounter) + "\n");
                }
                dstNoImmed = true;
                instruction = DIV * 8;
                if(sizeOfOperand == 'w') instruction += 4;
            }
            else if(!strcmp(tkn, "cmp")) {
                if(!regex_match(savedLine.c_str(), regex("(\\s*)cmp[ bw](\\s*)[^,]+(\\s*)[,](\\s*)[^,]+(\\s*)"))) {
                    throw runtime_error("Syntax error on line " + std::to_string(lineCounter) + "\n");
                }
                instruction = CMP * 8;
                if(sizeOfOperand == 'w') instruction += 4;
            }
            else if(!strcmp(tkn, "not")) {
                if(!regex_match(savedLine.c_str(), regex("(\\s*)not[ bw](\\s*)[^,]+(\\s*)[,](\\s*)[^,]+(\\s*)"))) {
                    throw runtime_error("Syntax error on line " + std::to_string(lineCounter) + "\n");
                }
                dstNoImmed = true;
                instruction = NOT * 8;
                if(sizeOfOperand == 'w') instruction += 4;
            }
            else if(!strcmp(tkn, "and")) {
                if(!regex_match(savedLine.c_str(), regex("(\\s*)and[ bw](\\s*)[^,]+(\\s*)[,](\\s*)[^,]+(\\s*)"))) {
                    throw runtime_error("Syntax error on line " + std::to_string(lineCounter) + "\n");
                }
                dstNoImmed = true;
                instruction = AND * 8;
                if(sizeOfOperand == 'w') instruction += 4;
            }
            else if(!strcmp(tkn, "or")) {
                if(!regex_match(savedLine.c_str(), regex("(\\s*)or[ bw](\\s*)[^,]+(\\s*)[,](\\s*)[^,]+(\\s*)"))) {
                    throw runtime_error("Syntax error on line " + std::to_string(lineCounter) + "\n");
                }
                dstNoImmed = true;
                instruction = OR * 8;
                if(sizeOfOperand == 'w') instruction += 4;
            }
            else if(!strcmp(tkn, "xor")) {
                if(!regex_match(savedLine.c_str(), regex("(\\s*)xor[ bw](\\s*)[^,]+(\\s*)[,](\\s*)[^,]+(\\s*)"))) {
                    throw runtime_error("Syntax error on line " + std::to_string(lineCounter) + "\n");
                }
                dstNoImmed = true;
                instruction = XOR * 8;
                if(sizeOfOperand == 'w') instruction += 4;
            }
            else if(!strcmp(tkn, "test")) {
                if(!regex_match(savedLine.c_str(), regex("(\\s*)test[ bw](\\s*)[^,]+(\\s*)[,](\\s*)[^,]+(\\s*)"))) {
                    throw runtime_error("Syntax error on line " + std::to_string(lineCounter) + "\n");
                }
                instruction = TST * 8;
                if(sizeOfOperand == 'w') instruction += 4;
            }
            else if(!strcmp(tkn, "shl")) {
                if(!regex_match(savedLine.c_str(), regex("(\\s*)shl[ bw](\\s*)[^,]+(\\s*)[,](\\s*)[^,]+(\\s*)"))) {
                    throw runtime_error("Syntax error on line " + std::to_string(lineCounter) + "\n");
                }
                dstNoImmed = true;
                instruction = SHL * 8;
                if(sizeOfOperand == 'w') instruction += 4;
            }
            else if(!strcmp(tkn, "shr")) {
                if(!regex_match(savedLine.c_str(), regex("(\\s*)shr[ bw](\\s*)[^,]+(\\s*)[,](\\s*)[^,]+(\\s*)"))) {
                    throw runtime_error("Syntax error on line " + std::to_string(lineCounter) + "\n");
                }
                dstNoImmed = true;
                instruction = SHR * 8;
                if(sizeOfOperand == 'w') instruction += 4;
            }
            ///blok 2
            string op1(strtok(0, delimiters));
            char* op2Tkn = strtok(0, delimiters);
            string op2;
            if(op2Tkn == NULL) op2 = " ";
            else op2 = op2Tkn;
            bool changeOffsetIfPCREL = false;
            //jumpovi
            if(isJump) { //jmpovi
                if(regex_match(op1.c_str(), regex("^(([0-9]+)|(0[bB][01]+)|(0[xX][a-fA-F0-9]+))$"))) {//literal, neposredno
                    instruction<<=8;//op1Descr sve nule
                    if(op1[0] == '0' && (op1[1]=='x' || op1[1]=='X')) { //hex number
                        op1 = op1.substr(2);
                        if(sizeOfOperand == 'w') { // ako je word
                            if(!regex_match(op1.c_str(), regex("^[0]*[a-fA-F0-9]{1,4}$")))  throw runtime_error("Syntax error on line " + std::to_string(lineCounter) + "\n");
                            if(op1[op1.length()-1] <= 122 && op1[op1.length()-1] >= 97) op1[op1.length()-1] -= 32; // to upper
                            if(op1[op1.length()-2] <= 122 && op1[op1.length()-2] >= 97) op1[op1.length()-2] -= 32;
                            if(op1[op1.length()-3] <= 122 && op1[op1.length()-3] >= 97) op1[op1.length()-3] -= 32;
                            if(op1[op1.length()-4] <= 122 && op1[op1.length()-4] >= 97) op1[op1.length()-4] -= 32;
                            //add content
                            instruction<<=8;// postavlja nizi bajt immed
                            instruction += (strtol(op1.c_str(), NULL, 16) & ((1<<8) - 1));
                            instruction<<=8;//postavlja visi bajt immed
                            instruction += (strtol(op1.c_str(), NULL, 16) >> 8);
                        }
                        else {// ako je byte
                            if(!regex_match(op1.c_str(), regex("0[xX][0]*[a-fA-F0-9]{1,2}"))) throw runtime_error("Syntax error on line " + std::to_string(lineCounter) + "\n");
                            if(op1[op1.length()-1] <= 122 && op1[op1.length()-1] >= 97) op1[op1.length()-1] -= 32;// to upper
                            if(op1[op1.length()-2] <= 122 && op1[op1.length()-2] >= 97) op1[op1.length()-2] -= 32;
                            //add content
                            instruction<<=8;// postavlja nizi bajt immed, ne treba and
                            instruction += strtol(op1.c_str(), NULL, 16);
                        }
                    }
                    else if(op1[0] == '0' && (op1[1]=='b' || op1[1]=='B')) { //bin number
                        op1 = op1.substr(2);
                        if(sizeOfOperand == 'w') { // ako je word
                            if(!regex_match(op1.c_str(), regex("^[0]*[01]{1,16}$")))  throw runtime_error("Syntax error on line " + std::to_string(lineCounter) + "\n");
                            //add content
                            instruction<<=8;// postavlja nizi bajt immed
                            instruction += (strtol(op1.c_str(), NULL, 2) & ((1<<8) - 1));
                            instruction<<=8;//postavlja visi bajt immed
                            instruction += (strtol(op1.c_str(), NULL, 2) >> 8);
                        }
                        else {// ako je byte
                            if(!regex_match(op1.c_str(), regex("^[0]*[01]{1,8}$"))) throw runtime_error("Syntax error on line " + std::to_string(lineCounter) + "\n");
                            //add content
                            instruction<<=8;// postavlja nizi bajt immed, ne treba and
                            instruction += strtol(op1.c_str(), NULL, 2);
                        }
                    }
                    else if (regex_match(op1.c_str(), regex("^(\\d+)$"))) { // dec
                        unsigned op1INT = atoi(op1.c_str());
                        if(sizeOfOperand == 'w') {//word
                            if(op1INT > 65535) throw runtime_error("Syntax error on line " + std::to_string(lineCounter) + "\n");
                            //add content
                            instruction<<=8;// postavlja nizi bajt immed
                            instruction += (op1INT & ((1<<8) - 1));
                            instruction<<=8;//postavlja visi bajt immed
                            instruction += (op1INT >> 8);
                        }
                        else {//byte
                            if(op1INT > 255 ) throw runtime_error("Syntax error on line " + std::to_string(lineCounter) + "\n");
                            //add content
                            instruction<<=8;// postavlja nizi bajt immed, ne treba and
                            instruction += op1INT;
                        }
                    }
                    else throw runtime_error("Syntax error on line " + std::to_string(lineCounter) + "\n");
                    //loc Counter, mozda jos nesto???
                    if(sizeOfOperand == 'w') locationCounter += 3;
                    else locationCounter += 2;
                }
                else if(regex_match(op1.c_str(), regex("^(\\w+)$"))) {// simbol, prosli rel i ref
                    instruction<<=8;
                    locationCounter++;
                    Symbol s;
                    s.name = op1;
                    s.global = false;
                    s.name = op1;
                    if(findSymbol(s.name)) {
                        Symbol* sPtr =getSymbol(s.name);

                        if(sPtr->global){//globalni simbol ili eksterni simbol
                            Relocation newRel;
                            newRel.id = sPtr->id;
                            newRel.offset = locationCounter;
                            newRel.section = currentSection->name;
                            newRel.type = "32";
                            //vrednost simbola je 00
                            currentSection->relTable.push_back(newRel);
                            instruction<<=8;
                            if(sizeOfOperand == 'w') instruction<<=8;
                        }
                        else {//lokalni simbol
                            Relocation newRel;
                            newRel.offset = locationCounter;
                            newRel.section = currentSection->name;
                            newRel.type = "32";

                            if(sPtr->defined) {
                                newRel.id = sPtr->sectionNumber;
                                //upisi vrednost
                                if(sizeOfOperand == 'w' && sPtr->offset > 65535 ) {//greska ako ne staje u 2B
                                    throw runtime_error("Operand has value bigger than one word" + to_string(lineCounter) + '\n');
                                }
                                if(sizeOfOperand == 'b' && sPtr->offset > 255 ) {//greska ako ne staje u 1B
                                    throw runtime_error("Operand has value bigger than one word" + to_string(lineCounter) + '\n');
                                }
                                if(sizeOfOperand == 'w') {
                                    instruction<<=8;//Immed nizi bajt
                                    instruction += (sPtr->offset & ((1<<8) - 1));
                                    instruction<<=8;//postavlja visi bajt immed
                                    instruction += (sPtr->offset >> 8);
                                }
                                else {
                                    instruction<<=8;//Immed nizi bajt
                                    instruction += sPtr->offset;
                                }
                            }
                            else {
                                newRel.id = 0;
                                forwardRefEntry f;
                                f.address = locationCounter;
                                f.byte = (sizeOfOperand == 'w' ? false: true);
                                f.section = currentSection;
                                sPtr->forwardRefTable.push_back(f);
                                instruction<<=8;
                                if(sizeOfOperand == 'w') instruction<<=8;    
                            }
                            currentSection->relTable.push_back(newRel);
                        }
                    } 
                    else {
                        s.defined = false;
                        s.sectionName = "";
                        s.sectionNumber = 0;
                        s.offset = 0;
                        s.id = numOfSimbols++;
                        Relocation newRel;
                        newRel.offset = locationCounter;
                        newRel.section = currentSection->name;
                        newRel.type = "32";

                        if(checkGlobAndExtern(s.name)) { // da li je global, ovo prolazi samo ako je global jer da je ekstern bio bi vec u tabeli
                            s.global = true;
                            //napravi rel zapis
                            newRel.id = s.id;
                        }
                        else {// lokalan
                            s.global = false;
                            //napravi rel zapis
                            newRel.id = 0;// ne zna se sekcija lokalnog simbola pa je u onim gore forama postavljamo
                            forwardRefEntry f;
                            f.address = locationCounter;
                            f.byte = (sizeOfOperand == 'w' ? false: true);
                            f.section = currentSection;
                            s.forwardRefTable.push_back(f);
                        }

                        currentSection->relTable.push_back(newRel);
                        tableOfSymbols.push_back(s);
                        instruction<<=8;
                        if(sizeOfOperand == 'w') instruction<<=8;
                    }
                    locationCounter++;
                    if(sizeOfOperand == 'w') locationCounter++;
                }
                else if(regex_match(op1.c_str(), regex("^\\*\\%r[0-7][hl]{0,1}$"))) { //op1 regdir
                    instruction<<=3;//op1Descr regdir
                    instruction+=1;
                    instruction<<=4;//op1Descr redni broj reg
                    instruction+=atoi(op1.substr(3,1).c_str());
                    instruction<<=1;
                    if(op1[op1.length() - 1] == 'h' && sizeOfOperand == 'b') instruction+=1;//postavljanje L/H bita op1Descr
                    if((op1[op1.length() - 1] == 'l' || op1[op1.length() - 1] == 'h') && sizeOfOperand == 'w' || (op1[op1.length() - 1] != 'l' && op1[op1.length() - 1] != 'h' && sizeOfOperand == 'b')) {//ako je velicina operanda 2B, a neko je za dst naveo 1B, i obrnuto, onda greska
                        throw runtime_error("Mismatch in operand size and instruction, on line " + to_string(lineCounter) + ".\n");
                    }
                    locationCounter++;
                }
                else if(regex_match(op1.c_str(), regex("^\\*\\(\\%r[0-7]\\)$"))) { //op1 regIndir
                    instruction<<=3;//op1Descr regIndir
                    instruction+=2;
                    instruction<<=4;//op1Descr redni broj reg
                    instruction+=atoi(op1.substr(4,1).c_str());
                    instruction<<=1;
                    if((op1[op1.length() - 2] == 'l' || op1[op1.length() - 2] == 'h') ) {//ako je velicina operanda 2B, a neko je za dst naveo 1B, onda greska
                        throw runtime_error("Not allowed to specify 8bits of register to use if not regDir, on line " + to_string(lineCounter) + ".\n");
                    }
                    locationCounter++;
                }
                else if(regex_match(op1.c_str(), regex("^\\*(([\\-]{0,1}[0-9]+)|(0[bB][0]*[01]+)|(0[xX][0]*[a-fA-F0-9]+))\\(\\%r[0-7]\\)$"))) {//op1 regIndirPom
                    instruction<<=3;//op1Descr regIndirPom
                    instruction+=3;
                    instruction<<=4;//op1Descr redni broj reg
                    instruction+=atoi(op1.substr(op1.find('r') + 1, 1).c_str());
                    instruction<<=1;
                    if((op1[op1.length() - 2] == 'l' || op1[op1.length() - 2] == 'h') ) {//ako je velicina operanda 2B, a neko je za dst naveo 1B, onda greska
                        throw runtime_error("Not allowed to specify 8bits of register to use if not regDir, on line " + to_string(lineCounter) + ".\n");
                    }
                    locationCounter+=3;//op1Descr + 2 bajta za pomeraj
                    //koji je broj
                    op1 = op1.substr(1, op1.find('(') - 1);//preskoci zvezdicu
                    if(op1[0] == '0' && (op1[1]=='x' || op1[1]=='X')) { //hex number
                        if(!regex_match(op1.c_str(), regex("0[xX][0]*[a-fA-F0-9]{1,4}")))  throw runtime_error("Syntax error, operand too big, on line " + std::to_string(lineCounter) + "\n");
                        op1 = op1.substr(2);
                        
                        if(op1[op1.length()-1] <= 122 && op1[op1.length()-1] >= 97) op1[op1.length()-1] -= 32; // to upper
                        if(op1[op1.length()-2] <= 122 && op1[op1.length()-2] >= 97) op1[op1.length()-2] -= 32;
                        if(op1[op1.length()-3] <= 122 && op1[op1.length()-3] >= 97) op1[op1.length()-3] -= 32;
                        if(op1[op1.length()-4] <= 122 && op1[op1.length()-4] >= 97) op1[op1.length()-4] -= 32;
                        //add content
                        instruction<<=8;// postavlja nizi bajt immed
                        instruction += (strtol(op1.c_str(), NULL, 16) & ((1<<8) - 1));
                        instruction<<=8;//postavlja visi bajt immed
                        instruction += (strtol(op1.c_str(), NULL, 16) >> 8);
                    }
                    else if(op1[0] == '0' && (op1[1]=='b' || op1[1]=='B')) { //bin number
                        if(!regex_match(op1.c_str(), regex("0[bB][0]*[0-1]{1,16}"))) throw runtime_error("Syntax error, operand too big, on line " + std::to_string(lineCounter) + "\n");
                        op1 = op1.substr(2);
                        //add content
                        instruction<<=8;// postavlja nizi bajt immed
                        instruction += (strtol(op1.c_str(), NULL, 2) & ((1<<8) - 1));
                        instruction<<=8;//postavlja visi bajt immed
                        instruction += (strtol(op1.c_str(), NULL, 2) >> 8);
                    }
                    else if (regex_match(op1.c_str(), regex("^[\\-]{0,1}(\\d+)$"))) { // dec
                        int op1INT = atoi(op1.c_str());
                        if(op1INT > 65535) throw runtime_error("Syntax error, operand too big, on line " + std::to_string(lineCounter) + "\n");
                        //add content
                        instruction<<=8;// postavlja nizi bajt immed
                        instruction += (op1INT & ((1<<8) - 1));
                        instruction<<=8;//postavlja visi bajt immed
                        instruction += ((op1INT >> 8) & ((1<<8) - 1));
                    }
                }
                else if(regex_match(op1.c_str(), regex("^\\*(\\w+)\\(((\\%r7)|(\\%pc))\\)$"))) {//op1 regIndirPom sa simbolom, za pc
                    instruction<<=3;//op1Descr regIndirPom
                    instruction+=3;
                    instruction<<=4;//op1Descr redni broj reg
                    instruction+=7;//pc je r7
                    instruction<<=1;
                    locationCounter++;
                    if((op1[op1.length() - 2] == 'l' || op1[op1.length() - 2] == 'h') ) {//ako je velicina operanda 2B, a neko je za dst naveo 1B, onda greska
                        throw runtime_error("Not allowed to specify 8bits of register to use if not regDir, on line " + to_string(lineCounter) + ".\n");
                    }
                    Symbol s;
                    s.name = op1;
                    s.global = false;
                    s.name = op1.substr(1, op1.find_first_of('(') - 1);
                    if(findSymbol(s.name)) {
                        Symbol* sPtr = getSymbol(s.name);

                        if(sPtr->defined){
                            if(sPtr->sectionName == currentSection->name) {
                                instruction<<=8;
                                instruction+= ((sPtr->offset - locationCounter - 2) & ((1<<8) - 1));// 
                                instruction<<=8;
                                instruction += (((long)(sPtr->offset - locationCounter - 2) >> 8) & ((1<<8) - 1));
                                if( (long)(sPtr->offset - locationCounter - 2) > 65535 ) throw runtime_error("Error on line" + to_string(lineCounter) + ", value of \"" + sPtr->name + " + LC - 2\" is too big to fit into 2B for PC-Rel.\n");
                            }
                            else {
                                Relocation newRel;
                                newRel.id = sPtr->id;
                                newRel.offset = locationCounter;
                                newRel.section = currentSection->name;
                                newRel.type = "PC32";
                                if(sPtr->global)   {//global
                                    currentSection->relTable.push_back(newRel);
                                    instruction<<=8;
                                    instruction += ((1<< 8) - 2);//FE
                                    instruction<<=8;
                                    instruction += ((1<<8) - 1);//FF
                                }
                                else {
                                    newRel.id = sPtr->sectionNumber;
                                    currentSection->relTable.push_back(newRel);
                                    instruction<<=8;
                                    instruction+= ((sPtr->offset - 2) & ((1<<8) - 1));// 
                                    instruction<<=8;
                                    instruction += (((long)(sPtr->offset - 2) >> 8) & ((1<<8) - 1));
                                    if( (long)(sPtr->offset - 2) > 65535 ) throw runtime_error("Error on line" + to_string(lineCounter) + ", value of symbol \"" + sPtr->name + "- 2\" is too big to fit into 2B for PC-Rel.\n");
                                }
                            }
                        }
                        else {//undefined simbol
                            if(!checkExtern(sPtr->name)) {
                                forwardRefEntry f;
                                f.section = currentSection;
                                f.byte = false;
                                f.address = locationCounter;
                                sPtr->forwardRefTable.push_back(f);
                            }
                            Relocation newRel;
                            newRel.id = sPtr->id;
                            newRel.offset = locationCounter;
                            newRel.section = currentSection->name;
                            newRel.type = "PC32";
                            
                            if(!sPtr->global) {
                                newRel.id = 0;//jos nije def lokalan simbol, pa u update symbol cemo odraditi ono azurirarnje rel.id
                            }
                            currentSection->relTable.push_back(newRel);
                            //popuni instr
                            instruction<<=8;
                            instruction+= ((1<< 8) - 2);//FE
                            instruction<<=8;
                            instruction+= ((1<<8) - 1);//FF
                        }
                    } 
                    else {
                        s.defined = false;
                        s.sectionName = "";
                        s.sectionNumber = 0;
                        s.offset = 0;
                        s.id = numOfSimbols++;
                        Relocation newRel;
                        newRel.offset = locationCounter;
                        newRel.section = currentSection->name;
                        newRel.type = "PC32";

                        if(checkGlobAndExtern(s.name)) { // da li je global, ovo prolazi samo ako je global jer da je ekstern bio bi vec u tabeli
                            s.global = true;
                            //napravi rel zapis
                            newRel.id = s.id;
                        }
                        else {// lokalan
                            s.global = false;
                            //napravi rel zapis
                            newRel.id = 0;// ne zna se sekcija lokalnog simbola pa je u onim gore forama postavljamo
                        }
                        forwardRefEntry f;
                        f.section = currentSection;
                        f.address = locationCounter;
                        f.byte = false;
                        s.forwardRefTable.push_back(f);
                        
                        currentSection->relTable.push_back(newRel);
                        tableOfSymbols.push_back(s);

                        instruction<<=8;
                        instruction+= ((1<< 8) - 2);//FE
                        instruction<<=8;
                        instruction+= ((1<<8) - 1);//FF
                    }
                    locationCounter += 2;
                }
                else if(regex_match(op1.c_str(), regex("^\\*(\\w+)\\(\\%r[0-6][hl]{0,1}\\)$"))) {//op1 regIndirPom sa simbolom
                    instruction<<=3;//op1Descr regIndirPom
                    instruction+=3;
                    instruction<<=4;//op1Descr redni broj reg
                    instruction+=atoi(op1.substr(op1.find('r') + 1, 1).c_str());
                    instruction<<=1;
                    locationCounter++;
                    if((op1[op1.length() - 2] == 'l' || op1[op1.length() - 2] == 'h') ) {//ako je velicina operanda 2B, a neko je za dst naveo 1B, onda greska
                        throw runtime_error("Not allowed to specify 8bits of register to use if not regDir, on line " + to_string(lineCounter) + ".\n");
                    }
                    Symbol s;
                    s.name = op1;
                    s.global = false;
                    s.name = op1.substr(1, op1.find_first_of('(') - 1);
                    if(findSymbol(s.name)) {
                        Symbol* sPtr =getSymbol(s.name);

                        if(sPtr->defined){
                                Relocation newRel;
                                newRel.id = sPtr->id;
                                newRel.offset = locationCounter;
                                newRel.section = currentSection->name;
                                newRel.type = "32";
                                if(sPtr->global)   {//global
                                    currentSection->relTable.push_back(newRel);
                                    instruction<<=8;
                                    instruction<<=8;
                                }
                                else {
                                    newRel.id = sPtr->sectionNumber;
                                    currentSection->relTable.push_back(newRel);
                                    instruction<<=8;
                                    instruction+= ((sPtr->offset ) & ((1<<8) - 1 ));// 
                                    instruction<<=8;
                                    instruction += (((long)(sPtr->offset ) >> 8) & ((1<<8) - 1));
                                    if( sPtr->offset > 65535 ) throw runtime_error("Error on line" + to_string(lineCounter) + ", value of symbol \"" + sPtr->name + "\" is too big to fit into 2B for RegInd with offset.\n");
                                }
                            
                        }
                        else {//undefined simbol
                            Relocation newRel;
                            newRel.id = sPtr->id;
                            newRel.offset = locationCounter;
                            newRel.section = currentSection->name;
                            newRel.type = "32";
                            
                            if(!sPtr->global) {
                                forwardRefEntry f;
                                f.section = currentSection;
                                f.byte = false;
                                f.address = locationCounter;
                                sPtr->forwardRefTable.push_back(f);

                                newRel.id = 0;//jos nije def lokalan simbol, pa u update symbol cemo odraditi ono azurirarnje rel.id
                            }
                            currentSection->relTable.push_back(newRel);
                            //popuni instr
                            instruction<<=8;
                            instruction<<=8;   
                        }
                    } 
                    else {
                        s.defined = false;
                        s.sectionName = "";
                        s.sectionNumber = 0;
                        s.offset = 0;
                        s.id = numOfSimbols++;
                        Relocation newRel;
                        newRel.offset = locationCounter;
                        newRel.section = currentSection->name;
                        newRel.type = "32";

                        if(checkGlobAndExtern(s.name)) { // da li je global, ovo prolazi samo ako je global jer da je ekstern bio bi vec u tabeli
                            s.global = true;
                            //napravi rel zapis
                            newRel.id = s.id;
                        }
                        else {// lokalan
                            s.global = false;
                            //napravi rel zapis
                            forwardRefEntry f;
                            f.section = currentSection;
                            f.address = locationCounter;
                            f.byte = false;
                            s.forwardRefTable.push_back(f);
                            newRel.id = 0;
                        }

                        currentSection->relTable.push_back(newRel);
                        tableOfSymbols.push_back(s);
                        instruction<<=8;
                        instruction<<=8;
                    }
                    locationCounter += 2;
                }
                else if(regex_match(op1.c_str(), regex("^\\*(([0-9]+)|(0[bB][0]*[01]+)|(0[xX][0]*[a-fA-F0-9]+))$"))) {//literal, apsolutno
                    instruction<<=3;
                    instruction+=4;
                    instruction<<=5;
                    locationCounter+=3;
                    op1 = op1.substr(1);
                    if(op1[0] == '0' && (op1[1]=='x' || op1[1]=='X')) { //hex number
                        if(!regex_match(op1.c_str(), regex("0[xX][0]*[a-fA-F0-9]{1,4}")))  throw runtime_error("Syntax error, operand too big, on line " + std::to_string(lineCounter) + "\n");
                        op1 = op1.substr(2);
                        if(op1[op1.length()-1] <= 122 && op1[op1.length()-1] >= 97) op1[op1.length()-1] -= 32; // to upper
                        if(op1[op1.length()-2] <= 122 && op1[op1.length()-2] >= 97) op1[op1.length()-2] -= 32;
                        if(op1[op1.length()-3] <= 122 && op1[op1.length()-3] >= 97) op1[op1.length()-3] -= 32;
                        if(op1[op1.length()-4] <= 122 && op1[op1.length()-4] >= 97) op1[op1.length()-4] -= 32;
                        //add content
                        instruction<<=8;// postavlja nizi bajt immed
                        instruction += (strtol(op1.c_str(), NULL, 16) & ((1<<8) - 1));
                        instruction<<=8;//postavlja visi bajt immed
                        instruction += (strtol(op1.c_str(), NULL, 16) >> 8);
                    }
                    else if(op1[0] == '0' && (op1[1]=='b' || op1[1]=='B')) { //bin number
                        if(!regex_match(op1.c_str(), regex("0[bB][0]*[0-1]{1,16}"))) throw runtime_error("Syntax error, operand too big, on line " + std::to_string(lineCounter) + "\n");
                        op1 = op1.substr(2);
                        //add content
                        instruction<<=8;// postavlja nizi bajt immed
                        instruction += (strtol(op1.c_str(), NULL, 2) & ((1<<8) - 1));
                        instruction<<=8;//postavlja visi bajt immed
                        instruction += (strtol(op1.c_str(), NULL, 2) >> 8);
                    }
                    else if (regex_match(op1.c_str(), regex("^(\\d+)$"))) { // dec
                        unsigned op1INT = atoi(op1.c_str());
                        if( op1INT > 65535 ) throw runtime_error("Error on line" + to_string(lineCounter) + ", value of is too big to fit into 2B.\n");
                        //add content
                        instruction<<=8;// postavlja nizi bajt immed
                        instruction += (op1INT & ((1<<8) - 1));
                        instruction<<=8;//postavlja visi bajt immed
                        instruction += (op1INT >> 8);
                    }
                    else throw runtime_error("Syntax error on line " + std::to_string(lineCounter) + "\n");
                }
                else if(regex_match(op1.c_str(), regex("^\\*(\\w+)$"))) {//simbol, apsolutno
                    instruction<<=3;
                    instruction+=4;
                    instruction<<=5;//op1Descr
                    //za simbole nesto
                    locationCounter++;
                    Symbol s;
                    s.name = op1.substr(1);
                    s.global = false;
                    if(findSymbol(s.name)) {
                        Symbol* sPtr =getSymbol(s.name);

                        if(sPtr->global){//globalni simbol ili eksterni simbol
                            Relocation newRel;
                            newRel.id = sPtr->id;
                            newRel.offset = locationCounter;
                            newRel.section = currentSection->name;
                            newRel.type = "32";
                            //vrednost simbola je 00
                            currentSection->relTable.push_back(newRel);
                            instruction<<=8;
                            instruction<<=8;
                        }
                        else {//lokalni simbol
                            Relocation newRel;
                            newRel.offset = locationCounter;
                            newRel.section = currentSection->name;
                            newRel.type = "32";

                            if(sPtr->defined) {
                                newRel.id = sPtr->sectionNumber;
                                //upisi vrednost
                                if( sPtr->offset > 65535 ) throw runtime_error("Error on line" + to_string(lineCounter) + ", value of symbol \"" + sPtr->name + "\" is too big to fit into 2B.\n");
                                instruction<<=8;//Immed nizi bajt
                                instruction += (sPtr->offset & ((1<<8) - 1));
                                instruction<<=8;//postavlja visi bajt immed
                                instruction += (sPtr->offset >> 8);
                            }
                            else {
                                newRel.id = 0;
                                forwardRefEntry f;
                                f.address = locationCounter;
                                f.byte = false;
                                f.section = currentSection;
                                sPtr->forwardRefTable.push_back(f);
                                instruction<<=8;
                                instruction<<=8;    
                            }
                            currentSection->relTable.push_back(newRel);
                        }
                    } 
                    else {
                        s.defined = false;
                        s.sectionName = "";
                        s.sectionNumber = 0;
                        s.offset = 0;
                        s.id = numOfSimbols++;
                        Relocation newRel;
                        newRel.offset = locationCounter;
                        newRel.section = currentSection->name;
                        newRel.type = "32";

                        if(checkGlobAndExtern(s.name)) { // da li je global, ovo prolazi samo ako je global jer da je ekstern bio bi vec u tabeli
                            s.global = true;
                            //napravi rel zapis
                            newRel.id = s.id;
                        }
                        else {// lokalan
                            s.global = false;
                            //napravi rel zapis
                            newRel.id = 0;// ne zna se sekcija lokalnog simbola pa je u onim gore forama postavljamo
                            forwardRefEntry f;
                            f.address = locationCounter;
                            f.byte = false;
                            f.section = currentSection;
                            s.forwardRefTable.push_back(f);
                        }

                        currentSection->relTable.push_back(newRel);
                        tableOfSymbols.push_back(s);
                        instruction<<=8;
                        instruction<<=8;
                    }
                    locationCounter += 2;
                }
                else {
                    throw runtime_error("Syntax error, operand isn't recognized, on line " + to_string(lineCounter) + ".\n");
                }
            }
            else { // sve ostale instrukcije
                if(regex_match(op1.c_str(), regex("^\\$(([0-9]+)|(0[bB][01]+)|(0[xX][a-fA-F0-9]+))$"))) {//literal
                    if(!strcmp(tkn, "shr")) {
                        throw runtime_error("Illegal dst operand for shr, on line"  + std::to_string(lineCounter) + "\n");
                    }
                    if(!strcmp(tkn, "xchg")) {
                        throw runtime_error("Illegal dst operand for xchg, on line"  + std::to_string(lineCounter) + "\n");
                    }
                    if(!strcmp(tkn, "pop")) {
                        throw runtime_error("Illegal dst operand for pop, on line"  + std::to_string(lineCounter) + "\n");
                    }
                    if(op1[1] == '0' && (op1[2]=='x' || op1[2]=='X')) { //hex number
                        op1 = op1.substr(3);
                        if(sizeOfOperand == 'w') { // ako je word
                            if(!regex_match(op1.c_str(), regex("^[0]*[a-fA-F0-9]{1,4}$")))  throw runtime_error("Syntax error on line " + std::to_string(lineCounter) + "\n");
                            if(op1[op1.length()-1] <= 122 && op1[op1.length()-1] >= 97) op1[op1.length()-1] -= 32; // to upper
                            if(op1[op1.length()-2] <= 122 && op1[op1.length()-2] >= 97) op1[op1.length()-2] -= 32;
                            if(op1[op1.length()-3] <= 122 && op1[op1.length()-3] >= 97) op1[op1.length()-3] -= 32;
                            if(op1[op1.length()-4] <= 122 && op1[op1.length()-4] >= 97) op1[op1.length()-4] -= 32;
                            //add content
                            instruction<<=8;// postavlja 1 bajt sve nule, to je onaj op1Descr
                            instruction<<=8;// postavlja nizi bajt immed
                            instruction += (strtol(op1.c_str(), NULL, 16) & ((1<<8) - 1));
                            instruction<<=8;//postavlja visi bajt immed
                            instruction += (strtol(op1.c_str(), NULL, 16) >> 8);
                        }
                        else {// ako je byte
                            if(!regex_match(op1.c_str(), regex("^[0]*[a-fA-F0-9]{1,2}$"))) throw runtime_error("Syntax error on line " + std::to_string(lineCounter) + "\n");
                            if(op1[op1.length()-1] <= 122 && op1[op1.length()-1] >= 97) op1[op1.length()-1] -= 32;// to upper
                            if(op1[op1.length()-2] <= 122 && op1[op1.length()-2] >= 97) op1[op1.length()-2] -= 32;
                            //add content
                            instruction<<=8;// postavlja 1 bajt sve nule, to je onaj op1Descr
                            instruction<<=8;// postavlja nizi bajt immed, ne treba and
                            instruction += strtol(op1.c_str(), NULL, 16);
                        }
                    }
                    else if(op1[1] == '0' && (op1[2]=='b' || op1[2]=='B')) { //bin number
                        op1 = op1.substr(3);
                        if(sizeOfOperand == 'w') { // ako je word
                            if(!regex_match(op1.c_str(), regex("^[0]*[01]{1,16}$")))  throw runtime_error("Syntax error on line " + std::to_string(lineCounter) + "\n");
                            //add content
                            instruction<<=8;// postavlja 1 bajt sve nule, to je onaj op1Descr
                            instruction<<=8;// postavlja nizi bajt immed
                            instruction += (strtol(op1.c_str(), NULL, 2) & ((1<<8) - 1));
                            instruction<<=8;//postavlja visi bajt immed
                            instruction += (strtol(op1.c_str(), NULL, 2) >> 8);
                        }
                        else {// ako je byte
                            if(!regex_match(op1.c_str(), regex("^[0]*[01]{1,8}$"))) throw runtime_error("Syntax error on line " + std::to_string(lineCounter) + "\n");
                            //add content
                            instruction<<=8;// postavlja 1 bajt sve nule, to je onaj op1Descr
                            instruction<<=8;// postavlja nizi bajt immed, ne treba and
                            instruction += strtol(op1.c_str(), NULL, 2);
                        }
                    }
                    else if (regex_match(op1.c_str(), regex("^\\$(\\d+)$"))) { // dec
                        op1 = op1.substr(1);
                        unsigned op1INT = atoi(op1.c_str());
                        if(sizeOfOperand == 'w') {//word
                            if(op1INT > 65535) throw runtime_error("Syntax error on line " + std::to_string(lineCounter) + "\n");
                            //add content
                            instruction<<=8;// postavlja 1 bajt sve nule, to je onaj op1Descr
                            instruction<<=8;// postavlja nizi bajt immed
                            instruction += (op1INT & ((1<<8) - 1));
                            instruction<<=8;//postavlja visi bajt immed
                            instruction += (op1INT >> 8);
                        }
                        else {//byte
                            if(op1INT > 255 ) throw runtime_error("Syntax error on line " + std::to_string(lineCounter) + "\n");
                            //add content
                            instruction<<=8;// postavlja 1 bajt sve nule, to je onaj op1Descr
                            instruction<<=8;// postavlja nizi bajt immed, ne treba and
                            instruction += op1INT;
                        }
                    }
                    else throw runtime_error("Syntax error on line " + std::to_string(lineCounter) + "\n");
                    //loc Counter, mozda jos nesto???
                    if(sizeOfOperand == 'w') locationCounter += 3;
                    else locationCounter += 2;
                }
                else if(regex_match(op1.c_str(), regex("^\\$(\\w+)$"))) {//$simbol, menjaj za rel tabele
                    if(!strcmp(tkn, "shr")) {
                        throw runtime_error("Illegal dst operand for shr, on line"  + std::to_string(lineCounter) + "\n");
                    }
                    if(!strcmp(tkn, "xchg")) {
                        throw runtime_error("Illegal dst operand for xchg, on line"  + std::to_string(lineCounter) + "\n");
                    }
                    if(!strcmp(tkn, "pop")) {
                        throw runtime_error("Illegal dst operand for pop, on line"  + std::to_string(lineCounter) + "\n");
                    }
                    instruction<<=8;
                    locationCounter++;
                    Symbol s;
                    s.name = op1.substr(1);//preskoci $ znak
                    s.global = false;
                    if(findSymbol(s.name)) {
                        Symbol* sPtr =getSymbol(s.name);

                        if(sPtr->global){//globalni simbol ili eksterni simbol
                            Relocation newRel;
                            newRel.id = sPtr->id;
                            newRel.offset = locationCounter;
                            newRel.section = currentSection->name;
                            newRel.type = "32";
                            //vrednost simbola je 00
                            currentSection->relTable.push_back(newRel);
                            instruction<<=8;
                            if(sizeOfOperand == 'w') instruction<<=8;
                        }
                        else {//lokalni simbol
                            Relocation newRel;
                            newRel.offset = locationCounter;
                            newRel.section = currentSection->name;
                            newRel.type = "32";

                            if(sPtr->defined) {
                                newRel.id = sPtr->sectionNumber;
                                //upisi vrednost
                                if(sizeOfOperand == 'w' && sPtr->offset > 65535 ) {//greska ako ne staje u 2B
                                    throw runtime_error("Operand has value bigger than one word" + to_string(lineCounter) + '\n');
                                }
                                if(sizeOfOperand == 'b' && sPtr->offset > 255 ) {//greska ako ne staje u 1B
                                    throw runtime_error("Operand has value bigger than one word" + to_string(lineCounter) + '\n');
                                }
                                if(sizeOfOperand == 'w') {
                                    instruction<<=8;//Immed nizi bajt
                                    instruction += (sPtr->offset & ((1<<8) - 1));
                                    instruction<<=8;//postavlja visi bajt immed
                                    instruction += (sPtr->offset >> 8);
                                }
                                else {
                                    instruction<<=8;//Immed nizi bajt
                                    instruction += sPtr->offset;
                                }
                            }
                            else {
                                newRel.id = 0;
                                forwardRefEntry f;
                                f.address = locationCounter;
                                f.byte = (sizeOfOperand == 'w' ? false: true);
                                f.section = currentSection;
                                sPtr->forwardRefTable.push_back(f);
                                instruction<<=8;
                                if(sizeOfOperand == 'w') instruction<<=8;    
                            }
                            currentSection->relTable.push_back(newRel);
                        }
                    } 
                    else {
                        s.defined = false;
                        s.sectionName = "";
                        s.sectionNumber = 0;
                        s.offset = 0;
                        s.id = numOfSimbols++;
                        Relocation newRel;
                        newRel.offset = locationCounter;
                        newRel.section = currentSection->name;
                        newRel.type = "32";

                        if(checkGlobAndExtern(s.name)) { // da li je global, ovo prolazi samo ako je global jer da je ekstern bio bi vec u tabeli
                            s.global = true;
                            //napravi rel zapis
                            newRel.id = s.id;
                        }
                        else {// lokalan
                            s.global = false;
                            //napravi rel zapis
                            newRel.id = 0;// ne zna se sekcija lokalnog simbola pa je u onim gore forama postavljamo
                            forwardRefEntry f;
                            f.address = locationCounter;
                            f.byte = (sizeOfOperand == 'w' ? false: true);
                            f.section = currentSection;
                            s.forwardRefTable.push_back(f);
                        }

                        currentSection->relTable.push_back(newRel);
                        tableOfSymbols.push_back(s);
                        instruction<<=8;
                        if(sizeOfOperand == 'w') instruction<<=8;
                    }
                    locationCounter++;
                    if(sizeOfOperand == 'w') locationCounter++;
                }
                else if(regex_match(op1.c_str(), regex("^\\%r[0-7][hl]{0,1}$"))) { //op1 regdir
                    instruction<<=3;//op1Descr regdir
                    instruction+=1;
                    instruction<<=4;//op1Descr redni broj reg
                    instruction+=atoi(op1.substr(2,1).c_str());
                    instruction<<=1;
                    if(op1[op1.length() - 1] == 'h' && sizeOfOperand == 'b') instruction+=1;//postavljanje L/H bita op1Descr
                    if((op1[op1.length() - 1] == 'l' || op1[op1.length() - 1] == 'h') && sizeOfOperand == 'w' || (op1[op1.length() - 1] != 'l' && op1[op1.length() - 1] != 'h' && sizeOfOperand == 'b')) {//ako je velicina operanda 2B, a neko je za dst naveo 1B, onda greska
                        throw runtime_error("Mismatch in operand size and instruction, on line " + to_string(lineCounter) + ".\n");
                    }
                    locationCounter++;
                }
                else if(regex_match(op1.c_str(), regex("^\\(\\%r[0-7]\\)$"))) { //op1 regIndir
                    instruction<<=3;//op1Descr regIndir
                    instruction+=2;
                    instruction<<=4;//op1Descr redni broj reg
                    instruction+=atoi(op1.substr(3,1).c_str());
                    instruction<<=1;
                    if((op1[op1.length() - 2] == 'l' || op1[op1.length() - 2] == 'h') ) {//ako je velicina operanda 2B, a neko je za dst naveo 1B, onda greska
                        throw runtime_error("Not allowed to specify 8bits of register to use if not regDir, on line " + to_string(lineCounter) + ".\n");
                    }
                    locationCounter++;
                }
                else if(regex_match(op1.c_str(), regex("^(([\\-]{0,1}[0-9]+)|(0[bB][0]*[01]+)|(0[xX][0]*[a-fA-F0-9]+))\\(\\%r[0-7]\\)$"))) {//op1 regIndirPom
                    instruction<<=3;//op1Descr regIndirPom
                    instruction+=3;
                    instruction<<=4;//op1Descr redni broj reg
                    instruction+=atoi(op1.substr(op1.find('r') + 1, 1).c_str());
                    instruction<<=1;
                    if((op1[op1.length() - 2] == 'l' || op1[op1.length() - 2] == 'h') ) {//ako je velicina operanda 2B, a neko je za dst naveo 1B, onda greska
                        throw runtime_error("Not allowed to specify 8bits of register to use if not regDir, on line " + to_string(lineCounter) + ".\n");
                    }
                    locationCounter+=3;//op1Descr + 2 bajta za pomeraj
                    //koji je broj
                    op1 = op1.substr(0, op1.find('('));
                    if(op1[0] == '0' && (op1[1]=='x' || op1[1]=='X')) { //hex number
                        if(!regex_match(op1.c_str(), regex("0[xX][0]*[a-fA-F0-9]{1,4}")))  throw runtime_error("Syntax error, operand too big, on line " + std::to_string(lineCounter) + "\n");
                        op1 = op1.substr(2);
                        if(op1[op1.length()-1] <= 122 && op1[op1.length()-1] >= 97) op1[op1.length()-1] -= 32; // to upper
                        if(op1[op1.length()-2] <= 122 && op1[op1.length()-2] >= 97) op1[op1.length()-2] -= 32;
                        if(op1[op1.length()-3] <= 122 && op1[op1.length()-3] >= 97) op1[op1.length()-3] -= 32;
                        if(op1[op1.length()-4] <= 122 && op1[op1.length()-4] >= 97) op1[op1.length()-4] -= 32;
                        //add content
                        instruction<<=8;// postavlja nizi bajt immed
                        instruction += (strtol(op1.c_str(), NULL, 16) & ((1<<8) - 1));
                        instruction<<=8;//postavlja visi bajt immed
                        instruction += (strtol(op1.c_str(), NULL, 16) >> 8);
                    }
                    else if(op1[0] == '0' && (op1[1]=='b' || op1[1]=='B')) { //bin number
                        if(!regex_match(op1.c_str(), regex("0[bB][0]*[0-1]{1,16}"))) throw runtime_error("Syntax error, operand too big, on line " + std::to_string(lineCounter) + "\n");
                        op1 = op1.substr(2);
                        //add content
                        instruction<<=8;// postavlja nizi bajt immed
                        instruction += (strtol(op1.c_str(), NULL, 2) & ((1<<8) - 1));
                        instruction<<=8;//postavlja visi bajt immed
                        instruction += (strtol(op1.c_str(), NULL, 2) >> 8);
                    }
                    else if (regex_match(op1.c_str(), regex("^[\\-]{0,1}(\\d+)$"))) { // dec
                        int op1INT = atoi(op1.c_str());
                        if(op1INT > 65535) throw runtime_error("Syntax error, operand too big, on line " + std::to_string(lineCounter) + "\n");
                        //add content
                        instruction<<=8;// postavlja nizi bajt immed
                        instruction += (op1INT & ((1<<8) - 1));
                        instruction<<=8;//postavlja visi bajt immed
                        instruction += ((op1INT >> 8) & ((1<<8) - 1));
                    }
                }
                else if(regex_match(op1.c_str(), regex("^(\\w+)\\(((\\%r7)|(\\%pc))\\)$"))) {//op1 regIndirPom sa simbolom, za pc
                    instruction<<=3;//op1Descr regIndirPom
                    instruction+=3;
                    instruction<<=4;//op1Descr redni broj reg
                    instruction+=7;//pc je r7
                    instruction<<=1;
                    locationCounter++;
                    if((op1[op1.length() - 2] == 'l' || op1[op1.length() - 2] == 'h') ) {//ako je velicina operanda 2B, a neko je za dst naveo 1B, onda greska
                        throw runtime_error("Not allowed to specify 8bits of register to use if not regDir, on line " + to_string(lineCounter) + ".\n");
                    }
                    Symbol s;
                    s.global = false;
                    s.name = op1.substr(0, op1.find_first_of('('));
                    if(findSymbol(s.name)) {
                        Symbol* sPtr = getSymbol(s.name);
                        changeOffsetIfPCREL = true;

                        if(sPtr->defined){
                            if(sPtr->sectionName == currentSection->name) {
                                instruction<<=8;
                                instruction+= ((sPtr->offset - locationCounter - 2) & ((1<<8) - 1));// 
                                instruction<<=8;
                                instruction += (((long)(sPtr->offset - locationCounter - 2) >> 8) & ((1<<8) - 1));
                                if( (long)(sPtr->offset - locationCounter - 2) > 65535 ) throw runtime_error("Error on line" + to_string(lineCounter) + ", value of \"" + sPtr->name + " - LC - 2/3/4/5\" is too big to fit into 2B for PC-Rel.\n");//optimalno bi bilo da tek kad saznamo koliki je pom(ovo 2/3/4/5) onda tek vidimo je l izraz staje u 2B ali jbg
                            }
                            else {
                                Relocation newRel;
                                newRel.id = sPtr->id;
                                newRel.offset = locationCounter;
                                newRel.section = currentSection->name;
                                newRel.type = "PC32";
                                if(sPtr->global)   {//global
                                    currentSection->relTable.push_back(newRel);
                                    instruction<<=8;
                                    instruction+= ((1<< 8) - 2);//FE
                                    instruction<<=8;
                                    instruction+= ((1<<8) - 1);//FF
                                }
                                else {
                                    newRel.id = sPtr->sectionNumber;
                                    currentSection->relTable.push_back(newRel);
                                    instruction<<=8;
                                    instruction+= ((sPtr->offset - 2) & ((1<<8) - 1));// 
                                    instruction<<=8;
                                    instruction += (((long)(sPtr->offset - 2) >> 8) & ((1<<8) - 1));
                                    if( (long)(sPtr->offset - 2) > 65535 ) throw runtime_error("Error on line" + to_string(lineCounter) + ", value of symbol \"" + sPtr->name + "- 2/3/4/5\" is too big to fit into 2B for PC-Rel.\n");
                                }
                            }
                        }
                        else {//undefined simbol
                            if(!checkExtern(sPtr->name)) {
                                forwardRefEntry f;
                                f.section = currentSection;
                                f.byte = false;
                                f.address = locationCounter;
                                sPtr->forwardRefTable.push_back(f);
                            }
                            Relocation newRel;
                            newRel.id = sPtr->id;
                            newRel.offset = locationCounter;
                            newRel.section = currentSection->name;
                            newRel.type = "PC32";
                            
                            if(!sPtr->global) {
                                newRel.id = 0;//jos nije def lokalan simbol, pa u update symbol cemo odraditi ono azurirarnje rel.id
                            }
                            currentSection->relTable.push_back(newRel);
                            //popuni instr
                            instruction<<=8;
                            instruction+= ((1<< 8) - 2);//FE
                            instruction<<=8;
                            instruction+= ((1<<8) - 1);//FF
                        }
                    } 
                    else {
                        s.defined = false;
                        s.sectionName = "";
                        s.sectionNumber = 0;
                        s.offset = 0;
                        s.id = numOfSimbols++;
                        Relocation newRel;
                        newRel.offset = locationCounter;
                        newRel.section = currentSection->name;
                        newRel.type = "PC32";
                        changeOffsetIfPCREL = true;

                        if(checkGlobAndExtern(s.name)) { // da li je global, ovo prolazi samo ako je global jer da je ekstern bio bi vec u tabeli
                            s.global = true;
                            //napravi rel zapis
                            newRel.id = s.id;
                        }
                        else {// lokalan
                            s.global = false;
                            //napravi rel zapis
                            newRel.id = 0;// ne zna se sekcija lokalnog simbola pa je u onim gore forama postavljamo
                        }
                        forwardRefEntry f;
                        f.section = currentSection;
                        f.address = locationCounter;
                        f.byte = false;
                        s.forwardRefTable.push_back(f);

                        currentSection->relTable.push_back(newRel);
                        tableOfSymbols.push_back(s);
                        instruction<<=8;
                        instruction+= ((1<< 8) - 2);//FE
                        instruction<<=8;
                        instruction+= ((1<<8) - 1);//FF
                    }
                    locationCounter+=2;
                }
                else if(regex_match(op1.c_str(), regex("^(\\w+)\\(\\%r[0-6]\\)$"))) {//op1 regIndirPom sa simbolom
                    instruction<<=3;//op1Descr regIndirPom
                    instruction+=3;
                    instruction<<=4;//op1Descr redni broj reg
                    instruction+=atoi(op1.substr(op1.find('r') + 1, 1).c_str());
                    instruction<<=1;
                    locationCounter++;
                    if((op1[op1.length() - 2] == 'l' || op1[op1.length() - 2] == 'h') ) {//ako je velicina operanda 2B, a neko je za dst naveo 1B, onda greska
                        throw runtime_error("Not allowed to specify 8bits of register to use if not regDir, on line " + to_string(lineCounter) + ".\n");
                    }
                    Symbol s;
                    s.global = false;
                    s.name = op1.substr(0, op1.find_first_of('('));
                    if(findSymbol(s.name)) {
                        Symbol* sPtr =getSymbol(s.name);

                        if(sPtr->defined){
                                Relocation newRel;
                                newRel.id = sPtr->id;
                                newRel.offset = locationCounter;
                                newRel.section = currentSection->name;
                                newRel.type = "32";
                                if(sPtr->global)   {//global
                                    currentSection->relTable.push_back(newRel);
                                    instruction<<=8;
                                    instruction<<=8;
                                }
                                else {
                                    newRel.id = sPtr->sectionNumber;
                                    currentSection->relTable.push_back(newRel);
                                    instruction<<=8;
                                    instruction+= ((sPtr->offset ) & ((1<<8) - 1 ));// 
                                    instruction<<=8;
                                    instruction += (((long)(sPtr->offset ) >> 8) & ((1<<8) - 1));
                                    if( sPtr->offset > 65535 ) throw runtime_error("Error on line" + to_string(lineCounter) + ", value of symbol \"" + sPtr->name + "\" is too big to fit into 2B for RegInd with offset.\n");
                                }
                            
                        }
                        else {//undefined simbol
                            Relocation newRel;
                            newRel.id = sPtr->id;
                            newRel.offset = locationCounter;
                            newRel.section = currentSection->name;
                            newRel.type = "32";
                            
                            if(!sPtr->global) {
                                forwardRefEntry f;
                                f.section = currentSection;
                                f.byte = false;
                                f.address = locationCounter;
                                sPtr->forwardRefTable.push_back(f);

                                newRel.id = 0;//jos nije def lokalan simbol, pa u update symbol cemo odraditi ono azurirarnje rel.id
                            }
                            currentSection->relTable.push_back(newRel);
                            //popuni instr
                            instruction<<=8;
                            instruction<<=8;
                        }
                    } 
                    else {
                        s.defined = false;
                        s.sectionName = "";
                        s.sectionNumber = 0;
                        s.offset = 0;
                        s.id = numOfSimbols++;
                        Relocation newRel;
                        newRel.offset = locationCounter;
                        newRel.section = currentSection->name;
                        newRel.type = "32";

                        if(checkGlobAndExtern(s.name)) { // da li je global, ovo prolazi samo ako je global jer da je ekstern bio bi vec u tabeli
                            s.global = true;
                            //napravi rel zapis
                            newRel.id = s.id;
                        }
                        else {// lokalan
                            s.global = false;
                            //napravi rel zapis
                            forwardRefEntry f;
                            f.section = currentSection;
                            f.address = locationCounter;
                            f.byte = false;
                            s.forwardRefTable.push_back(f);
                            newRel.id = 0;
                        }

                        currentSection->relTable.push_back(newRel);
                        tableOfSymbols.push_back(s);
                        instruction<<=8;
                        instruction<<=8;
                    }
                    locationCounter+=2;
                }
                else if(regex_match(op1.c_str(), regex("^(([0-9]+)|(0[bB][0]*[01]+)|(0[xX][0]*[a-fA-F0-9]+))$"))) {//literal, apsolutno
                    instruction<<=3;
                    instruction+=4;
                    instruction<<=5;
                    locationCounter+=3;
                    if(op1[0] == '0' && (op1[1]=='x' || op1[1]=='X')) { //hex number
                        if(!regex_match(op1.c_str(), regex("0[xX][0]*[a-fA-F0-9]{1,4}")))  throw runtime_error("Syntax error, operand too big, on line " + std::to_string(lineCounter) + "\n");
                        op1 = op1.substr(2);
                        if(op1[op1.length()-1] <= 122 && op1[op1.length()-1] >= 97) op1[op1.length()-1] -= 32; // to upper
                        if(op1[op1.length()-2] <= 122 && op1[op1.length()-2] >= 97) op1[op1.length()-2] -= 32;
                        if(op1[op1.length()-3] <= 122 && op1[op1.length()-3] >= 97) op1[op1.length()-3] -= 32;
                        if(op1[op1.length()-4] <= 122 && op1[op1.length()-4] >= 97) op1[op1.length()-4] -= 32;
                        //add content
                        instruction<<=8;// postavlja nizi bajt immed
                        instruction += (strtol(op1.c_str(), NULL, 16) & ((1<<8) - 1));
                        instruction<<=8;//postavlja visi bajt immed
                        instruction += (strtol(op1.c_str(), NULL, 16) >> 8);
                    }
                    else if(op1[0] == '0' && (op1[1]=='b' || op1[1]=='B')) { //bin number
                        if(!regex_match(op1.c_str(), regex("0[bB][0]*[0-1]{1,16}"))) throw runtime_error("Syntax error, operand too big, on line " + std::to_string(lineCounter) + "\n");
                        op1 = op1.substr(2);
                        //add content
                        instruction<<=8;// postavlja nizi bajt immed
                        instruction += (strtol(op1.c_str(), NULL, 2) & ((1<<8) - 1));
                        instruction<<=8;//postavlja visi bajt immed
                        instruction += (strtol(op1.c_str(), NULL, 2) >> 8);
                    }
                    else if (regex_match(op1.c_str(), regex("^(\\d+)$"))) { // dec
                        unsigned op1INT = atoi(op1.c_str());
                        if(op1INT > 65535) throw runtime_error("Syntax error on line " + std::to_string(lineCounter) + "\n");
                        //add content
                        instruction<<=8;// postavlja nizi bajt immed
                        instruction += (op1INT & ((1<<8) - 1));
                        instruction<<=8;//postavlja visi bajt immed
                        instruction += (op1INT >> 8);
                    }
                    else throw runtime_error("Syntax error on line " + std::to_string(lineCounter) + "\n");
                }
                else if(regex_match(op1.c_str(), regex("^(\\w+)$"))) {//simbol, apsolutno
                    instruction<<=3;
                    instruction+=4;
                    instruction<<=5;
                    //za simbole nesto
                    locationCounter++;
                    Symbol s;
                    s.name = op1;
                    s.global = false;
                    if(findSymbol(s.name)) {
                        Symbol* sPtr =getSymbol(s.name);

                        if(sPtr->global){//globalni simbol ili eksterni simbol
                            Relocation newRel;
                            newRel.id = sPtr->id;
                            newRel.offset = locationCounter;
                            newRel.section = currentSection->name;
                            newRel.type = "32";
                            //vrednost simbola je 00
                            currentSection->relTable.push_back(newRel);
                            instruction<<=8;
                            instruction<<=8;
                        }
                        else {//lokalni simbol
                            Relocation newRel;
                            newRel.offset = locationCounter;
                            newRel.section = currentSection->name;
                            newRel.type = "32";

                            if(sPtr->defined) {
                                newRel.id = sPtr->sectionNumber;
                                //upisi vrednost
                                if( sPtr->offset > 65535 ) throw runtime_error("Error on line" + to_string(lineCounter) + ", value of symbol \"" + sPtr->name + "\" is too big to fit into 2B for RegInd with offset.\n");
                                instruction<<=8;//Immed nizi bajt
                                instruction += (sPtr->offset & ((1<<8) - 1));
                                instruction<<=8;//postavlja visi bajt immed
                                instruction += (sPtr->offset >> 8);
                            }
                            else {
                                newRel.id = 0;
                                forwardRefEntry f;
                                f.section = currentSection;
                                f.address = locationCounter;
                                f.byte = false;
                                sPtr->forwardRefTable.push_back(f);
                                instruction<<=8;
                                instruction<<=8;    
                            }
                            currentSection->relTable.push_back(newRel);
                        }
                    } 
                    else {
                        s.defined = false;
                        s.sectionName = "";
                        s.sectionNumber = 0;
                        s.offset = 0;
                        s.id = numOfSimbols++;
                        Relocation newRel;
                        newRel.offset = locationCounter;
                        newRel.section = currentSection->name;
                        newRel.type = "32";

                        if(checkGlobAndExtern(s.name)) { // da li je global, ovo prolazi samo ako je global jer da je ekstern bio bi vec u tabeli
                            s.global = true;
                            //napravi rel zapis
                            newRel.id = s.id;
                        }
                        else {// lokalan
                            s.global = false;
                            //napravi rel zapis
                            newRel.id = 0;// ne zna se sekcija lokalnog simbola pa je u onim gore forama postavljamo
                            forwardRefEntry f;
                            f.address = locationCounter;
                            f.byte = false;
                            f.section = currentSection;
                            s.forwardRefTable.push_back(f);
                        }

                        currentSection->relTable.push_back(newRel);
                        tableOfSymbols.push_back(s);
                        instruction<<=8;
                        instruction<<=8;
                    }
                    locationCounter += 2;
                }
                else {
                    throw runtime_error("Syntax error, operand isn't recognized, on line " + to_string(lineCounter) + ".\n");
                }

            }//6C800000
            //
            if(!singleOperand) {
                int oldLC = locationCounter;
                //
                if(regex_match(op2.c_str(), regex("^\\$(([0-9]+)|(0[bB][01]+)|(0[xX][a-fA-F0-9]+))$"))) {//literal     
                    if(strcmp(tkn, "shr")) {
                        throw runtime_error("Illegal dst operand on line"  + std::to_string(lineCounter) + "\n");
                    }
                    if(op2[1] == '0' && (op2[2]=='x' || op2[2]=='X')) { //hex number
                        op2 = op2.substr(3);
                        if(sizeOfOperand == 'w') { // ako je word
                            if(!regex_match(op2.c_str(), regex("^[0]*[a-fA-F0-9]{1,4}$")))  throw runtime_error("Syntax error on line " + std::to_string(lineCounter) + "\n");
                            if(op2[op2.length()-1] <= 122 && op2[op2.length()-1] >= 97) op2[op2.length()-1] -= 32; // to upper
                            if(op2[op2.length()-2] <= 122 && op2[op2.length()-2] >= 97) op2[op2.length()-2] -= 32;
                            if(op2[op2.length()-3] <= 122 && op2[op2.length()-3] >= 97) op2[op2.length()-3] -= 32;
                            if(op2[op2.length()-4] <= 122 && op2[op2.length()-4] >= 97) op2[op2.length()-4] -= 32;
                            //add content
                            instruction<<=8;// postavlja 1 bajt sve nule, to je onaj op1Descr
                            instruction<<=8;// postavlja nizi bajt immed
                            instruction += (strtol(op2.c_str(), NULL, 16) & ((1<<8) - 1));
                            instruction<<=8;//postavlja visi bajt immed
                            instruction += (strtol(op2.c_str(), NULL, 16) >> 8);
                        }
                        else {// ako je byte
                            if(!regex_match(op2.c_str(), regex("0[xX][0]*[a-fA-F0-9]{1,2}"))) throw runtime_error("Syntax error on line " + std::to_string(lineCounter) + "\n");
                            if(op2[op2.length()-1] <= 122 && op2[op2.length()-1] >= 97) op2[op2.length()-1] -= 32;// to upper
                            if(op2[op2.length()-2] <= 122 && op2[op2.length()-2] >= 97) op2[op2.length()-2] -= 32;
                            //add content
                            instruction<<=8;// postavlja 1 bajt sve nule, to je onaj op1Descr
                            instruction<<=8;// postavlja nizi bajt immed, ne treba and
                            instruction += strtol(op2.c_str(), NULL, 16);
                        }
                    }
                    else if(op2[1] == '0' && (op2[2]=='b' || op2[2]=='B')) { //bin number
                        op2 = op2.substr(3);
                        if(sizeOfOperand == 'w') { // ako je word
                            if(!regex_match(op2.c_str(), regex("^[0]*[01]{1,16}$")))  throw runtime_error("Syntax error on line " + std::to_string(lineCounter) + "\n");
                            //add content
                            instruction<<=8;// postavlja 1 bajt sve nule, to je onaj op1Descr
                            instruction<<=8;// postavlja nizi bajt immed
                            instruction += (strtol(op2.c_str(), NULL, 2) & ((1<<8) - 1));
                            instruction<<=8;//postavlja visi bajt immed
                            instruction += (strtol(op2.c_str(), NULL, 2) >> 8);
                        }
                        else {// ako je byte
                            if(!regex_match(op2.c_str(), regex("^[0]*[01]{1,8}$"))) throw runtime_error("Syntax error on line " + std::to_string(lineCounter) + "\n");
                            //add content
                            instruction<<=8;// postavlja 1 bajt sve nule, to je onaj op1Descr
                            instruction<<=8;// postavlja nizi bajt immed, ne treba and
                            instruction += strtol(op2.c_str(), NULL, 2);
                        }
                    }
                    else if (regex_match(op2.c_str(), regex("^\\$(\\d+)$"))) { // dec
                        op2 = op2.substr(1);
                        unsigned op1INT = atoi(op2.c_str());
                        if(sizeOfOperand == 'w') {//word
                            if(op1INT > 65535) throw runtime_error("Syntax error on line " + std::to_string(lineCounter) + "\n");
                            //add content
                            instruction<<=8;// postavlja 1 bajt sve nule, to je onaj op1Descr
                            instruction<<=8;// postavlja nizi bajt immed
                            instruction += (op1INT & ((1<<8) - 1));
                            instruction<<=8;//postavlja visi bajt immed
                            instruction += (op1INT >> 8);
                        }
                        else {//byte
                            if(op1INT > 255 ) throw runtime_error("Syntax error on line " + std::to_string(lineCounter) + "\n");
                            //add content
                            instruction<<=8;// postavlja 1 bajt sve nule, to je onaj op1Descr
                            instruction<<=8;// postavlja nizi bajt immed, ne treba and
                            instruction += op1INT;
                        }
                    }
                    else throw runtime_error("Syntax error on line " + std::to_string(lineCounter) + "\n");
                    //loc Counter, mozda jos nesto???
                    if(sizeOfOperand == 'w') locationCounter += 3;
                    else locationCounter += 2;
                }
                else if(regex_match(op2.c_str(), regex("^\\$(\\w+)$"))) {//$simbol, menjaj za rel tabele
                    if(strcmp(tkn, "shr")) {
                        throw runtime_error("Illegal dst operand on line"  + std::to_string(lineCounter) + "\n");
                    }
                    instruction<<=8;
                    locationCounter++;
                    Symbol s;
                    s.name = op2.substr(1);//skinemo $ znak
                    s.global = false;
                    if(findSymbol(s.name)) {
                        Symbol* sPtr =getSymbol(s.name);

                        if(sPtr->global){//globalni simbol ili eksterni simbol
                            Relocation newRel;
                            newRel.id = sPtr->id;
                            newRel.offset = locationCounter;
                            newRel.section = currentSection->name;
                            newRel.type = "32";
                            //vrednost simbola je 00
                            currentSection->relTable.push_back(newRel);
                            instruction<<=8;
                            if(sizeOfOperand == 'w') instruction<<=8;
                        }
                        else {//lokalni simbol
                            Relocation newRel;
                            newRel.offset = locationCounter;
                            newRel.section = currentSection->name;
                            newRel.type = "32";

                            if(sPtr->defined) {
                                newRel.id = sPtr->sectionNumber;
                                //upisi vrednost
                                if(sizeOfOperand == 'w' && sPtr->offset > 65535 ) {//greska ako ne staje u 2B
                                    throw runtime_error("Operand has value bigger than one word" + to_string(lineCounter) + '\n');
                                }
                                if(sizeOfOperand == 'b' && sPtr->offset > 255 ) {//greska ako ne staje u 1B
                                    throw runtime_error("Operand has value bigger than one word" + to_string(lineCounter) + '\n');
                                }
                                if(sizeOfOperand == 'w') {
                                    instruction<<=8;//Immed nizi bajt
                                    instruction += (sPtr->offset & ((1<<8) - 1));
                                    instruction<<=8;//postavlja visi bajt immed
                                    instruction += (sPtr->offset >> 8);
                                }
                                else {
                                    instruction<<=8;//Immed nizi bajt
                                    instruction += sPtr->offset;
                                }
                            }
                            else {
                                newRel.id = 0;
                                forwardRefEntry f;
                                f.address = locationCounter;
                                f.byte = (sizeOfOperand == 'w' ? false: true);
                                f.section = currentSection;
                                sPtr->forwardRefTable.push_back(f);
                                instruction<<=8;
                                if(sizeOfOperand == 'w') instruction<<=8;    
                            }
                            currentSection->relTable.push_back(newRel);
                        }
                    } 
                    else {
                        s.defined = false;
                        s.sectionName = "";
                        s.sectionNumber = 0;
                        s.offset = 0;
                        s.id = numOfSimbols++;
                        Relocation newRel;
                        newRel.offset = locationCounter;
                        newRel.section = currentSection->name;
                        newRel.type = "32";

                        if(checkGlobAndExtern(s.name)) { // da li je global, ovo prolazi samo ako je global jer da je ekstern bio bi vec u tabeli
                            s.global = true;
                            //napravi rel zapis
                            newRel.id = s.id;
                        }
                        else {// lokalan
                            s.global = false;
                            //napravi rel zapis
                            newRel.id = 0;// ne zna se sekcija lokalnog simbola pa je u onim gore forama postavljamo
                            forwardRefEntry f;
                            f.address = locationCounter;
                            f.byte = (sizeOfOperand == 'w' ? false: true);
                            f.section = currentSection;
                            s.forwardRefTable.push_back(f);
                        }

                        currentSection->relTable.push_back(newRel);
                        tableOfSymbols.push_back(s);
                        instruction<<=8;
                        if(sizeOfOperand == 'w') instruction<<=8;
                    }
                    locationCounter++;
                    if(sizeOfOperand == 'w') locationCounter++;
                }
                else if(regex_match(op2.c_str(), regex("^\\%r[0-7][hl]{0,1}$"))) { //op2 regdir
                    instruction<<=3;//op1Descr regdir
                    instruction+=1;
                    instruction<<=4;//op1Descr redni broj reg
                    instruction+=atoi(op2.substr(2,1).c_str());
                    instruction<<=1;
                    if(op2[op2.length() - 1] == 'h' && sizeOfOperand == 'b') instruction+=1;//postavljanje L/H bita op1Descr
                    if((op2[op2.length() - 1] == 'l' || op2[op2.length() - 1] == 'h') && sizeOfOperand == 'w' || (op2[op2.length() - 1] != 'l' && op2[op2.length() - 1] != 'h' && sizeOfOperand == 'b')) {//ako je velicina operanda 2B, a neko je za dst naveo 1B, onda greska
                        throw runtime_error("Mismatch in operand size and instruction, on line " + to_string(lineCounter) + ".\n");
                    }
                    locationCounter++;
                }
                else if(regex_match(op2.c_str(), regex("^\\(\\%r[0-7]\\)$"))) { //op2 regIndir
                    instruction<<=3;//op1Descr regIndir
                    instruction+=2;
                    instruction<<=4;//op1Descr redni broj reg
                    instruction+=atoi(op2.substr(3,1).c_str());
                    instruction<<=1;
                    if((op2[op2.length() - 2] == 'l' || op2[op2.length() - 2] == 'h') ) {//ako je velicina operanda 2B, a neko je za dst naveo 1B, onda greska
                        throw runtime_error("Not allowed to specify 8bits of register to use if not regDir, on line " + to_string(lineCounter) + ".\n");
                    }
                    locationCounter++;
                }
                else if(regex_match(op2.c_str(), regex("^(([\\-]{0,1}[0-9]+)|(0[bB][0]*[01]{1,16})|(0[xX][0]*[a-fA-F0-9]{1,4}))\\(\\%r[0-7]\\)$"))) {//op2 regIndirPom
                    instruction<<=3;//op1Descr regIndirPom
                    instruction+=3;
                    instruction<<=4;//op1Descr redni broj reg
                    instruction+=atoi(op2.substr(op2.find('r') + 1, 1).c_str());
                    instruction<<=1;
                    if((op2[op2.length() - 2] == 'l' || op2[op2.length() - 2] == 'h') ) {//ako je velicina operanda 2B, a neko je za dst naveo 1B, onda greska
                        throw runtime_error("Not allowed to specify 8bits of register to use if not regDir, on line " + to_string(lineCounter) + ".\n");
                    }
                    locationCounter+=3;//op1Descr + 2 bajta za pomeraj
                    //koji je broj
                    op2 = op2.substr(0, op2.find('('));
                    if(op2[0] == '0' && (op2[1]=='x' || op2[1]=='X')) { //hex number
                        if(!regex_match(op2.c_str(), regex("0[xX][0]*[a-fA-F0-9]{1,4}")))  throw runtime_error("Syntax error, operand too big, on line " + std::to_string(lineCounter) + "\n");
                        op2 = op2.substr(2);
                        if(op2[op2.length()-1] <= 122 && op2[op2.length()-1] >= 97) op2[op2.length()-1] -= 32; // to upper
                        if(op2[op2.length()-2] <= 122 && op2[op2.length()-2] >= 97) op2[op2.length()-2] -= 32;
                        if(op2[op2.length()-3] <= 122 && op2[op2.length()-3] >= 97) op2[op2.length()-3] -= 32;
                        if(op2[op2.length()-4] <= 122 && op2[op2.length()-4] >= 97) op2[op2.length()-4] -= 32;
                        //add content
                        instruction<<=8;// postavlja nizi bajt immed
                        instruction += (strtol(op2.c_str(), NULL, 16) & ((1<<8) - 1));
                        instruction<<=8;//postavlja visi bajt immed
                        instruction += (strtol(op2.c_str(), NULL, 16) >> 8);
                    }
                    else if(op2[0] == '0' && (op2[1]=='b' || op2[1]=='B')) { //bin number
                        if(!regex_match(op2.c_str(), regex("0[bB][0]*[0-1]{1,16}"))) throw runtime_error("Syntax error, operand too big, on line " + std::to_string(lineCounter) + "\n");
                        op2 = op2.substr(2);
                        //add content
                        instruction<<=8;// postavlja nizi bajt immed
                        instruction += (strtol(op2.c_str(), NULL, 2) & ((1<<8) - 1));
                        instruction<<=8;//postavlja visi bajt immed
                        instruction += (strtol(op2.c_str(), NULL, 2) >> 8);
                    }
                    else if (regex_match(op2.c_str(), regex("^[\\-]{0,1}(\\d+)$"))) { // dec
                        int op1INT = atoi(op2.c_str());
                        if(op1INT > 65535) throw runtime_error("Syntax error, operand too big, on line " + std::to_string(lineCounter) + "\n");
                        //add content
                        instruction<<=8;// postavlja nizi bajt immed
                        instruction += (op1INT & ((1<<8) - 1));
                        instruction<<=8;//postavlja visi bajt immed
                        instruction += ((op1INT >> 8) & ((1<<8) - 1));
                    }
                }
                else if(regex_match(op2.c_str(), regex("^(\\w+)\\(((\\%r7)|(\\%pc))\\)$"))) {//op2 regIndirPom sa simbolom, za pc, ovde si menjao gledaj
                    instruction<<=3;//op1Descr regIndirPom
                    instruction+=3;
                    instruction<<=4;//op1Descr redni broj reg
                    instruction+=7;//pc je r7
                    instruction<<=1;
                    locationCounter++;
                    if((op2[op2.length() - 2] == 'l' || op2[op2.length() - 2] == 'h') ) {//ako je velicina operanda 2B, a neko je za dst naveo 1B, onda greska
                        throw runtime_error("Not allowed to specify 8bits of register to use if not regDir, on line " + to_string(lineCounter) + ".\n");
                    }
                    //nesto duradimo za simbol
                    Symbol s;
                    s.global = false;
                    s.name = op2.substr(0, op2.find_first_of('('));
                    if(findSymbol(s.name)) {
                        Symbol* sPtr =getSymbol(s.name);

                        if(sPtr->defined){
                            if(sPtr->sectionName == currentSection->name) {
                                instruction<<=8;
                                instruction+= ((sPtr->offset - locationCounter - 2) & ((1<<8) - 1));// 
                                instruction<<=8;
                                instruction += (((long)(sPtr->offset - locationCounter - 2) >> 8) & ((1<<8) - 1));
                                if( (long)(sPtr->offset - locationCounter - 2) > 65535 ) throw runtime_error("Error on line" + to_string(lineCounter) + ", value of \"" + sPtr->name + " - LC - 2\" is too big to fit into 2B for PC-Rel.\n");
                            }
                            else {
                                Relocation newRel;
                                newRel.id = sPtr->id;
                                newRel.offset = locationCounter;
                                newRel.section = currentSection->name;
                                newRel.type = "PC32";
                                if(sPtr->global)   {//global
                                    currentSection->relTable.push_back(newRel);
                                    instruction<<=8;
                                    instruction+= ((1<< 8) - 2);//FE
                                    instruction<<=8;
                                    instruction+= ((1<<8) - 1);//FF
                                }
                                else {
                                    newRel.id = sPtr->sectionNumber;
                                    currentSection->relTable.push_back(newRel);
                                    instruction<<=8;
                                    instruction+= ((sPtr->offset - 2) & ((1<<8) - 1));// 
                                    instruction<<=8;
                                    instruction += (((long)(sPtr->offset - 2) >> 8) & ((1<<8) - 1));
                                    if( (long)(sPtr->offset - 2) > 65535 ) throw runtime_error("Error on line" + to_string(lineCounter) + ", value of symbol \"" + sPtr->name + "- 2\" is too big to fit into 2B for PC-Rel.\n");
                                }
                            }
                        }
                        else {//undefined simbol
                            if(!checkExtern(sPtr->name)) {
                                forwardRefEntry f;
                                f.section = currentSection;
                                f.byte = false;
                                f.address = locationCounter;
                                sPtr->forwardRefTable.push_back(f);
                            }
                            Relocation newRel;
                            newRel.id = sPtr->id;
                            newRel.offset = locationCounter;
                            newRel.section = currentSection->name;
                            newRel.type = "PC32";
                            
                            if(!sPtr->global) {
                                newRel.id = 0;//jos nije def lokalan simbol, pa u update symbol cemo odraditi ono azurirarnje rel.id
                            }
                            currentSection->relTable.push_back(newRel);
                            //popuni instr
                            instruction<<=8;
                            instruction+= ((1<< 8) - 2);//FE
                            instruction<<=8;
                            instruction+= ((1<<8) - 1);//FF
                        }
                    } 
                    else {
                        s.defined = false;
                        s.sectionName = "";
                        s.sectionNumber = 0;
                        s.offset = 0;
                        s.id = numOfSimbols++;
                        Relocation newRel;
                        newRel.offset = locationCounter;
                        newRel.section = currentSection->name;
                        newRel.type = "PC32";

                        if(checkGlobAndExtern(s.name)) { // da li je global, ovo prolazi samo ako je global jer da je ekstern bio bi vec u tabeli
                            s.global = true;
                            //napravi rel zapis
                            newRel.id = s.id;
                        }
                        else {// lokalan
                            s.global = false;
                            //napravi rel zapis
                            newRel.id = 0;// ne zna se sekcija lokalnog simbola pa je u onim gore forama postavljamo
                        }
                        forwardRefEntry f;
                        f.section = currentSection;
                        f.address = locationCounter;
                        f.byte = false;
                        s.forwardRefTable.push_back(f);

                        currentSection->relTable.push_back(newRel);
                        tableOfSymbols.push_back(s);
                        instruction<<=8;
                        instruction+= ((1<< 8) - 2);//FE
                        instruction<<=8;
                        instruction+= ((1<<8) - 1);//FF
                    }
                    locationCounter += 2;
                }
                else if(regex_match(op2.c_str(), regex("^(\\w+)\\(\\%r[0-6]\\)$"))) {//op2 regIndirPom sa simbolom
                    instruction<<=3;//op1Descr regIndirPom
                    instruction+=3;
                    instruction<<=4;//op1Descr redni broj reg
                    instruction+=atoi(op2.substr(op2.find('r') + 1, 1).c_str());
                    instruction<<=1;
                    locationCounter++;
                    if((op2[op2.length() - 2] == 'l' || op2[op2.length() - 2] == 'h') ) {//ako je velicina operanda 2B, a neko je za dst naveo 1B, onda greska
                        throw runtime_error("Not allowed to specify 8bits of register to use if not regDir, on line " + to_string(lineCounter) + ".\n");
                    }
                    //nesto duradimo za simbol
                    Symbol s;
                    s.global = false;
                    s.name = op2.substr(0, op2.find_first_of('('));
                    if(findSymbol(s.name)) {
                        Symbol* sPtr =getSymbol(s.name);

                        if(sPtr->defined){
                                Relocation newRel;
                                newRel.id = sPtr->id;
                                newRel.offset = locationCounter;
                                newRel.section = currentSection->name;
                                newRel.type = "32";
                                if(sPtr->global)   {//global
                                    currentSection->relTable.push_back(newRel);
                                    instruction<<=8;
                                    instruction<<=8;
                                }
                                else {
                                    newRel.id = sPtr->sectionNumber;
                                    currentSection->relTable.push_back(newRel);
                                    instruction<<=8;
                                    instruction+= ((sPtr->offset ) & ((1<<8) - 1 ));// 
                                    instruction<<=8;
                                    instruction += (((long)(sPtr->offset ) >> 8) & ((1<<8) - 1));
                                    if( sPtr->offset > 65535 ) throw runtime_error("Error on line" + to_string(lineCounter) + ", value of symbol \"" + sPtr->name + "\" is too big to fit into 2B for RegInd with offset.\n");
                                }
                        }
                        else {//undefined simbol
                            Relocation newRel;
                            newRel.id = sPtr->id;
                            newRel.offset = locationCounter;
                            newRel.section = currentSection->name;
                            newRel.type = "32";
                            
                            if(!sPtr->global) {
                                forwardRefEntry f;
                                f.section = currentSection;
                                f.byte = false;
                                f.address = locationCounter;
                                sPtr->forwardRefTable.push_back(f);

                                newRel.id = 0;//jos nije def lokalan simbol, pa u update symbol cemo odraditi ono azurirarnje rel.id
                            }
                            currentSection->relTable.push_back(newRel);
                            //popuni instr
                            instruction<<=8;
                            instruction<<=8;
                        }
                    } 
                    else {
                        s.defined = false;
                        s.sectionName = "";
                        s.sectionNumber = 0;
                        s.offset = 0;
                        s.id = numOfSimbols++;
                        Relocation newRel;
                        newRel.offset = locationCounter;
                        newRel.section = currentSection->name;
                        newRel.type = "32";

                        if(checkGlobAndExtern(s.name)) { // da li je global, ovo prolazi samo ako je global jer da je ekstern bio bi vec u tabeli
                            s.global = true;
                            //napravi rel zapis
                            newRel.id = s.id;
                        }
                        else {// lokalan
                            s.global = false;
                            //napravi rel zapis
                            forwardRefEntry f;
                            f.section = currentSection;
                            f.address = locationCounter;
                            f.byte = false;
                            s.forwardRefTable.push_back(f);
                            newRel.id = 0;
                        }

                        currentSection->relTable.push_back(newRel);
                        tableOfSymbols.push_back(s);
                        instruction<<=8;
                        instruction<<=8;
                    }
                    locationCounter += 2;
                }
                else if(regex_match(op2.c_str(), regex("^(([0-9]+)|(0[bB][0]*[01]{1,16})|(0[xX][0]*[a-fA-F0-9]{1,4}))$"))) {//literal, apsolutno
                    instruction<<=3;
                    instruction+=4;
                    instruction<<=5;
                    locationCounter+=3;
                    if(op2[0] == '0' && (op2[1]=='x' || op2[1]=='X')) { //hex number
                        if(!regex_match(op2.c_str(), regex("0[xX][0]*[a-fA-F0-9]{1,4}")))  throw runtime_error("Syntax error, operand too big, on line " + std::to_string(lineCounter) + "\n");
                        op2 = op2.substr(2);
                        if(op2[op2.length()-1] <= 122 && op2[op2.length()-1] >= 97) op2[op2.length()-1] -= 32; // to upper
                        if(op2[op2.length()-2] <= 122 && op2[op2.length()-2] >= 97) op2[op2.length()-2] -= 32;
                        if(op2[op2.length()-3] <= 122 && op2[op2.length()-3] >= 97) op2[op2.length()-3] -= 32;
                        if(op2[op2.length()-4] <= 122 && op2[op2.length()-4] >= 97) op2[op2.length()-4] -= 32;
                        //add content
                        instruction<<=8;// postavlja nizi bajt immed
                        instruction += (strtol(op2.c_str(), NULL, 16) & ((1<<8) - 1));
                        instruction<<=8;//postavlja visi bajt immed
                        instruction += (strtol(op2.c_str(), NULL, 16) >> 8);
                    }
                    else if(op2[0] == '0' && (op2[1]=='b' || op2[1]=='B')) { //bin number
                        if(!regex_match(op2.c_str(), regex("0[bB][0]*[0-1]{1,16}"))) throw runtime_error("Syntax error, operand too big, on line " + std::to_string(lineCounter) + "\n");
                        op2 = op2.substr(2);
                        //add content
                        instruction<<=8;// postavlja nizi bajt immed
                        instruction += (strtol(op2.c_str(), NULL, 2) & ((1<<8) - 1));
                        instruction<<=8;//postavlja visi bajt immed
                        instruction += (strtol(op2.c_str(), NULL, 2) >> 8);
                    }
                    else if (regex_match(op2.c_str(), regex("^(\\d+)$"))) { // dec
                        unsigned op1INT = atoi(op2.c_str());
                        if(op1INT > 65535) throw runtime_error("Syntax error on line " + std::to_string(lineCounter) + "\n");
                        //add content
                        instruction<<=8;// postavlja nizi bajt immed
                        instruction += (op1INT & ((1<<8) - 1));
                        instruction<<=8;//postavlja visi bajt immed
                        instruction += (op1INT >> 8);
                    }
                    else throw runtime_error("Syntax error on line " + std::to_string(lineCounter) + "\n");
                }
                else if(regex_match(op2.c_str(), regex("^(\\w+)$"))) {//simbol, apsolutno
                    instruction<<=3;
                    instruction+=4;
                    instruction<<=5;
                    //za simbole nesto
                    locationCounter++;
                    Symbol s;
                    s.name = op2;
                    s.global = false;
                    if(findSymbol(s.name)) {
                        Symbol* sPtr =getSymbol(s.name);

                        if(sPtr->global){//globalni simbol ili eksterni simbol
                            Relocation newRel;
                            newRel.id = sPtr->id;
                            newRel.offset = locationCounter;
                            newRel.section = currentSection->name;
                            newRel.type = "32";
                            //vrednost simbola je 00
                            currentSection->relTable.push_back(newRel);
                            instruction<<=8;
                            instruction<<=8;
                        }
                        else {//lokalni simbol
                            Relocation newRel;
                            newRel.offset = locationCounter;
                            newRel.section = currentSection->name;
                            newRel.type = "32";

                            if(sPtr->defined) {
                                newRel.id = sPtr->sectionNumber;
                                //upisi vrednost
                                if( sPtr->offset > 65535 ) throw runtime_error("Error on line" + to_string(lineCounter) + ", value of symbol \"" + sPtr->name + "\" is too big to fit into 2B.\n");
                                instruction<<=8;//Immed nizi bajt
                                instruction += (sPtr->offset & ((1<<8) - 1));
                                instruction<<=8;//postavlja visi bajt immed
                                instruction += (sPtr->offset >> 8);
                            }
                            else {
                                newRel.id = 0;
                                forwardRefEntry f;
                                f.address = locationCounter;
                                f.byte = false;
                                f.section = currentSection;
                                sPtr->forwardRefTable.push_back(f);
                                instruction<<=8;
                                instruction<<=8;    
                            }
                            currentSection->relTable.push_back(newRel);
                        }
                    } 
                    else {
                        s.defined = false;
                        s.sectionName = "";
                        s.sectionNumber = 0;
                        s.offset = 0;
                        s.id = numOfSimbols++;
                        Relocation newRel;
                        newRel.offset = locationCounter;
                        newRel.section = currentSection->name;
                        newRel.type = "32";

                        if(checkGlobAndExtern(s.name)) { // da li je global, ovo prolazi samo ako je global jer da je ekstern bio bi vec u tabeli
                            s.global = true;
                            //napravi rel zapis
                            newRel.id = s.id;
                        }
                        else {// lokalan
                            s.global = false;
                            //napravi rel zapis
                            newRel.id = 0;// ne zna se sekcija lokalnog simbola pa je u onim gore forama postavljamo
                            forwardRefEntry f;
                            f.address = locationCounter;
                            f.byte = false;
                            f.section = currentSection;
                            s.forwardRefTable.push_back(f);
                        }

                        currentSection->relTable.push_back(newRel);
                        tableOfSymbols.push_back(s);
                        instruction<<=8;
                        instruction<<=8;
                    }
                    locationCounter += 2;
                }
                else {
                    throw runtime_error("Syntax error, operand isn't recognized, on line " + to_string(lineCounter) + ".\n");
                }
                //
                oldLC -= locationCounter;
                if(changeOffsetIfPCREL) {//podesavanje pomeraja za pcRel
                    char* p = (char*) &instruction;
                    p += (-oldLC);// vratis se na bajt koji predstavlja offset koji se treba promeniti
                    int oldOffset = (((int)*p) & ((1 << 8) - 1));// uzmemo da izracunamo vrednost tako sto pokupimo staru pa dodamo oldLC
                    oldOffset <<= 8;
                    p++;
                    oldOffset += (((int)*p) & ((1 << 8) - 1));
                    oldOffset += oldLC;
                    //
                    char* p2 = (char*)&oldOffset;//novi pokazivac na ovu izracunatu vrednost
                    *p = *(p2); // upises nizi bajt
                    *(p-1) = *(p2 + 1); // upises visi bajt
                }
            }
            //dodavanje u section content
            string toAdd = decimalToHexa((char*)to_string(instruction).c_str(), 16);
            for(int i = toAdd.length() - 1; i >= 1; i-=2){
                currentSection->content+= toAdd[i];
                currentSection->content+= toAdd[i - 1];
                currentSection->content+= " ";
            }
            //next
            assemblerStatus = END_LINE;
            lineCounter++;
            continue;
        }
        else {
            throw runtime_error("Illegal instruction \"" + string(tkn) + "\".");
        }
    }
    //equ backpatching
    for(int i = 0; i < EQUTable.size(); i++) { // svaki put kad definisemo novi EQU simbol proverimo je l moze sad neki, koji ranije nismo uspeli da izracunamo, da izracunamo
        vector<string> sections = vector<string>();//
        vector<int> values = vector<int>();//      
        long literalValues = 0;
        bool skip = false;
        for(int j = 0; j < EQUTable[i].table.size(); j++) {
            //prolazimo kroz svaki simbol u izrazu
            if(!strcmp(EQUTable[i].table[j].section.c_str(), "") && strcmp(EQUTable[i].table[j].symbolName.c_str(), "")) {//
                //da li je def u tabeliSimbola
                if(getSymbol(EQUTable[i].table[j].symbolName) && getSymbol(EQUTable[i].table[j].symbolName)->defined) {
                    EQUTable[i].table[j].section = getSymbol(EQUTable[i].table[j].symbolName)->sectionName;
                    if(!strcmp(EQUTable[i].table[j].section.c_str(), "ABS")) EQUTable[i].table[j].value *= getSymbol(EQUTable[i].table[j].symbolName)->offset;//ako je naknadno dodat ABS sekcija, onda treba podesiti i value kao literalu
                }
                else {
                    //throw runtime_error("Undefined symbol \"" + EQUTable[i].table[j].symbolName + "\", in EQU expression for symbol \"" + EQUTable[i].symbol  + "\".\n");
                   skip = true;
                   break;
                };
            }
            //
            if((!strcmp(EQUTable[i].table[j].section.c_str(), "") && !strcmp(EQUTable[i].table[j].symbolName.c_str(), "")) || !strcmp(EQUTable[i].table[j].section.c_str(), "ABS")) {//ako je literal
                literalValues += EQUTable[i].table[j].value;
                continue;
            }
            int index = -1;
            for(int k = 0; k < sections.size(); k++) {
                if(!strcmp(sections[k].c_str(), EQUTable[i].table[j].section.c_str())) {
                    index = k;
                }
            }
            if(index != -1) {
                values[index] += EQUTable[i].table[j].value;
            }
            else {
                sections.push_back(EQUTable[i].table[j].section);
                values.push_back(EQUTable[i].table[j].value);
            }
        }
        //
        if(skip) continue;
        int foundOne = -1;
        for(int j = 0; j < sections.size(); j++) {
            //printf("Sections i value: %s i %d, za %s\n", sections[j].c_str(), values[j], EQUTable[i].symbol.c_str());
            if((foundOne != -1 && values[j] != 0) || (values[j] != 0 && values[j] != 1)) throw runtime_error("EQU statement for symbol \"" + EQUTable[i].symbol + "\" not valid.\n");
            if(values[j] == 1) foundOne = j;
        }
        Symbol toInsert;
        toInsert.defined = true;
        toInsert.id = numOfSimbols++;
        toInsert.name = EQUTable[i].symbol;
        toInsert.global = false;
        if(checkGlobAndExtern(toInsert.name)) toInsert.global = true;//proveri je l globalan, ako ima neki eksterni
        toInsert.offset = literalValues;
        if(foundOne == -1) {
            toInsert.sectionName = "ABS";
            toInsert.sectionNumber = 1;
        }
        else {
            toInsert.sectionName = sections[foundOne];
            toInsert.sectionNumber = getSymbol(toInsert.sectionName)->id;
        }
        //racunanje toInsert.offset
        for(int j = 0; j < EQUTable[i].table.size(); j++) {
            if(strcmp(EQUTable[i].table[j].symbolName.c_str(), "") && strcmp(EQUTable[i].table[j].section.c_str(), "ABS")) {
                if(EQUTable[i].table[j].value == 1) toInsert.offset += getSymbol(EQUTable[i].table[j].symbolName)->offset;
                else toInsert.offset -= getSymbol(EQUTable[i].table[j].symbolName)->offset;
            }
        }
        //ubaci simbol u tabelu
        if(getSymbol(toInsert.name)) {
            updateSymbol(toInsert);
            getSymbol(toInsert.name)->sectionName = toInsert.sectionName;//u update symbol se uvek postavlja sectionName na currSection->name, pa ko zna sta bi se desilo kad bi to menjao pa cu ovako
            getSymbol(toInsert.name)->sectionNumber = toInsert.sectionNumber;
        }
        else  tableOfSymbols.push_back(toInsert);
        //
        EQUTable.erase(EQUTable.begin() + i);
        i = -1;
    }
    //Da li ima nedefinisanih EQU simbola
    if(EQUTable.size() > 0) {
        throw runtime_error("Can't calculate the value of EQU symbol \"" + EQUTable.front().symbol  + "\".\n");
    }
    //
    //
    for(int i = 0; i < tableOfSymbols.size(); i++) {//backpatching
        if(tableOfSymbols[i].name == tableOfSymbols[i].sectionName) continue;

        if(!tableOfSymbols[i].defined && strcmp(tableOfSymbols[i].sectionName.c_str(), "UND")) {
            throw runtime_error("Symbol: \"" + tableOfSymbols[i].name + "\" not defined");
        }

        for(int j = 0; j < tableOfSymbols[i].forwardRefTable.size(); j++) {
            for(int k = 0; k < tableOfSymbols[i].forwardRefTable[j].section->relTable.size() ; k++) {
                if(tableOfSymbols[i].forwardRefTable[j].address != tableOfSymbols[i].forwardRefTable[j].section->relTable[k].offset ) {
                    continue;
                }
                if(!tableOfSymbols[i].global && !strcmp(tableOfSymbols[i].forwardRefTable[j].section->relTable[k].type.c_str(), "32")) {
                    string symValue = decimalToHexa((char*)to_string(tableOfSymbols[i].offset).c_str(), 4);
                    if(tableOfSymbols[i].forwardRefTable[j].byte == 1) {
                        if(symValue.length() > 1){
                            tableOfSymbols[i].forwardRefTable[j].section->content[tableOfSymbols[i].forwardRefTable[j].address*3] = symValue[1];
                        }
                        tableOfSymbols[i].forwardRefTable[j].section->content[tableOfSymbols[i].forwardRefTable[j].address*3 + 1] = symValue[0];
                    }
                    else {
                        switch (symValue.length())
                        {
                        case 4:
                            tableOfSymbols[i].forwardRefTable[j].section->content[tableOfSymbols[i].forwardRefTable[j].address*3 + 4] = symValue[3];
                        case 3:
                            tableOfSymbols[i].forwardRefTable[j].section->content[tableOfSymbols[i].forwardRefTable[j].address*3 + 3] = symValue[2];
                        case 2:
                            tableOfSymbols[i].forwardRefTable[j].section->content[tableOfSymbols[i].forwardRefTable[j].address*3] = symValue[1];
                        case 1:
                            tableOfSymbols[i].forwardRefTable[j].section->content[tableOfSymbols[i].forwardRefTable[j].address*3 + 1] = symValue[0];
                            break;
                        default:
                            throw runtime_error("Default branch in backpatching.");
                            break;
                        }
                    }
                }
                else if(!strcmp(tableOfSymbols[i].forwardRefTable[j].section->relTable[k].type.c_str(), "PC32")) {
                    long toInsert;// bilo unsigned
                    if(!strcmp(tableOfSymbols[i].sectionName.c_str(), tableOfSymbols[i].forwardRefTable[j].section->name.c_str())) {
                        toInsert = tableOfSymbols[i].offset - tableOfSymbols[i].forwardRefTable[j].address;

                        tableOfSymbols[i].forwardRefTable[j].section->relTable[k].id = -1;// brisanje rel zapisa             
                    }
                    else if(!tableOfSymbols[i].global) {
                        toInsert = tableOfSymbols[i].offset;

                        tableOfSymbols[i].forwardRefTable[j].section->relTable[k].id = tableOfSymbols[i].sectionNumber; // premesti rel zapis da ukazuje na sekciju u kojoj je def simbol                        
                    }
                    else {
                        continue;
                    }
                    //procitaj sadrzaj sa te lok i dodaj na toInsert
                    string relContent = "";
                    relContent += tableOfSymbols[i].forwardRefTable[j].section->content[tableOfSymbols[i].forwardRefTable[j].address*3];
                    relContent += tableOfSymbols[i].forwardRefTable[j].section->content[tableOfSymbols[i].forwardRefTable[j].address*3 + 1];
                    //
                    toInsert += (strtol(relContent.c_str(), NULL, 16) | ~((1 << 8) - 1));// sign extend za neg broj, a ovo sto izvucemo iz content je uvek negativno jer PC32
                    std::stringstream ss;
                    //sjajno
                    ss << std::uppercase << std::hex << toInsert;
                    string res (ss.str());
                    if(res.length() > 4) res = res.substr(res.length() - 4, 4);
                    if(tableOfSymbols[i].forwardRefTable[j].byte == 1) {
                        if(res.length() > 1){
                            tableOfSymbols[i].forwardRefTable[j].section->content[tableOfSymbols[i].forwardRefTable[j].address*3] = res[1];
                            tableOfSymbols[i].forwardRefTable[j].section->content[tableOfSymbols[i].forwardRefTable[j].address*3 + 1] = res[0];
                        }
                    }
                    else {
                        switch (res.length())
                        {
                        case 4:
                            tableOfSymbols[i].forwardRefTable[j].section->content[tableOfSymbols[i].forwardRefTable[j].address*3] = res[2];
                            tableOfSymbols[i].forwardRefTable[j].section->content[tableOfSymbols[i].forwardRefTable[j].address*3 + 1] = res[3];
                            tableOfSymbols[i].forwardRefTable[j].section->content[tableOfSymbols[i].forwardRefTable[j].address*3 + 3] = res[0];
                            tableOfSymbols[i].forwardRefTable[j].section->content[tableOfSymbols[i].forwardRefTable[j].address*3 + 4] = res[1];
                            break;
                        case 3:
                            tableOfSymbols[i].forwardRefTable[j].section->content[tableOfSymbols[i].forwardRefTable[j].address*3] = res[1];
                            tableOfSymbols[i].forwardRefTable[j].section->content[tableOfSymbols[i].forwardRefTable[j].address*3 + 1] = res[2];
                            tableOfSymbols[i].forwardRefTable[j].section->content[tableOfSymbols[i].forwardRefTable[j].address*3 + 3] = '0';
                            tableOfSymbols[i].forwardRefTable[j].section->content[tableOfSymbols[i].forwardRefTable[j].address*3 + 4] = res[0];
                            break;
                        case 2:
                            tableOfSymbols[i].forwardRefTable[j].section->content[tableOfSymbols[i].forwardRefTable[j].address*3] = res[0];
                            tableOfSymbols[i].forwardRefTable[j].section->content[tableOfSymbols[i].forwardRefTable[j].address*3 + 1] = res[1];
                            tableOfSymbols[i].forwardRefTable[j].section->content[tableOfSymbols[i].forwardRefTable[j].address*3 + 3] = '0';
                            tableOfSymbols[i].forwardRefTable[j].section->content[tableOfSymbols[i].forwardRefTable[j].address*3 + 4] = '0';
                            break;
                        case 1:
                            tableOfSymbols[i].forwardRefTable[j].section->content[tableOfSymbols[i].forwardRefTable[j].address*3] = '0';
                            tableOfSymbols[i].forwardRefTable[j].section->content[tableOfSymbols[i].forwardRefTable[j].address*3 + 1] = res[0];
                            tableOfSymbols[i].forwardRefTable[j].section->content[tableOfSymbols[i].forwardRefTable[j].address*3 + 3] = '0';
                            tableOfSymbols[i].forwardRefTable[j].section->content[tableOfSymbols[i].forwardRefTable[j].address*3 + 4] = '0';
                            break;
                        default:
                            throw runtime_error("Default branch in backpatching.");
                            break;
                        }
                    }
                }
                else throw runtime_error("Unrecognized relocation type.");
            }
        }
    }
    
    //ispis u izlazniFajl
    //tabelaSimbola
    *out << "#tabela simbola" << "\n";
    *out << "#ime --- sekcija --- vrednost --- vidljivost --- redni broj" << "\n";
    stringstream ss;
    for(int i = 0; i < tableOfSymbols.size(); i++) {
        ss.str("");
        ss << uppercase << hex << tableOfSymbols[i].offset;
        *out << tableOfSymbols[i].name << " --- " << tableOfSymbols[i].sectionName << " --- " << ss.str() << " --- ";
        if(tableOfSymbols[i].global) *out << "globalan --- ";
        else *out << "lokalan --- ";
        ss.str("");
        ss << uppercase << hex << tableOfSymbols[i].id;
        *out << ss.str() << "\n";
    }
    //sve rel tabele svih sekcija
    for(int i = 0; i < sections.size(); i++) {
        *out << "\n#Relokaciona tabela: " << sections[i].name << "\n";
        *out << "#adresa --- tip --- ID simbola" << "\n";
        for(int j = 0; j < sections[i].relTable.size(); j++) {
            if(sections[i].relTable[j].id != -1) {
                ss.str("");
                ss << uppercase << hex << sections[i].relTable[j].offset;
                *out << ss.str() << " --- " << sections[i].relTable[j].type << " --- ";
                ss.str("");
                ss << uppercase << hex << sections[i].relTable[j].id;
                *out << ss.str() << "\n";
            }
        }
    }
    //sadrzaj 
    for(int i = 0; i < sections.size(); i++) {
        *out << "\n#" << sections[i].name << "\n";
        *out << sections[i].content << "\n";
    }
}

