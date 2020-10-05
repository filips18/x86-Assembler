#ifndef _H_EUNMC_H
#define _H_ENUMC_H

enum opCodes {
    HALT = 0,
    IRET = 1,
    RET = 2,
    INT = 3,
    CALL = 4,
    JMP = 5,
    JEQ = 6,
    JNE = 7,
    JGT = 8,
    PUSH = 9,
    POP = 10,
    XCHG = 11,
    MOV = 12,
    ADD = 13,
    SUB = 14,
    MUL = 15,
    DIV = 16,
    CMP = 17,
    NOT = 18,
    AND = 19,
    OR = 20,
    XOR = 21,
    TST = 22,
    SHL = 23,
    SHR = 24
};

enum states {
    START,
    START_LINE,
    END,
    END_LINE,
    LABEL,
    DIRECTIVE,
    INSTRUCTION,
    SECTION
};

#endif