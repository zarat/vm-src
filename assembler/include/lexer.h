#ifndef LEXER_H_INCLUDED
#define LEXER_H_INCLUDED

#include <string>
#include <cstdio>

enum {
    END = 0,
    MOV, 
    PUSH,
    POP, 
    LDR, 
    STR,
    LDM,
    STM,
    LDMR,
    STMR,
    ADD, // 10
    ADDI, 
    SUB, 
    MUL, 
    DIV,
    MOD,
    EQ,
    LT, 
    GT,
    LEQ,
    GEQ, // 20
    JMP,  
    JZ,
    JNZ, 
    RET,
    PRINT, 
    PRINTC,  
    READ, 
    WRITE, 
    PUTS,
    GETS, // 30
    READC,
    CMP,
    PRC,
    SI,
    INC,
    DEC,
    CALL,
    INT,
    /* 
    Internal opcodes    
    */ 
    AX, 
    BX, 
    CX, 
    DX, 
    R1,
    R2,
    R3,
    R4,
    R5,
    R6,
    R7,
    R8,
    R9,
    R10, 
    // LEXER INTERNALS
    EOL, 
    LABEL, 
    COLON, 
    INTEGER,
    STRING
};

class Lexer {

    FILE * f;    
    int last;
    int readChar();     
       
    public:   
        Lexer(const std::string & fname);
        ~Lexer();
    
    int alreadyread = 0;
    int getToken();
    int peek();
    
    std::string lastIdentifier;
    unsigned int lastInteger; 
    std::string lastString; // deprecated
    int lastToken;
    
};
#endif