#include "../h/Assembler.h"
#include <fstream>

int main(int argc, const char* argv[]) {
    if(argc < 4 || strcmp(argv[1], "-o")) {
        printf("Please call the program as: ./exe -o outputFile inputFile\n");
        return 1;
    }
    
    Assembler asmb;

    ifstream inputFile;
    ofstream outputFile;
    inputFile.open(argv[3]);
    outputFile.open(argv[2]);

    if(!inputFile.is_open() || !outputFile.is_open()) {
        printf("Error while opening files.\n");
        return 1;
    }

    asmb.start(&inputFile, &outputFile);
    return 0;
}