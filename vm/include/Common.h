#include <string.h>

/*
Common enumerations for opcodes
*/
enum {
    END, // = 0
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
    INTEGER
};

char *int2bin(int n) {
    unsigned i;
    char *test = (char *)malloc(37);
    memset(test, 0, 37);
    int c = 1;
    for (i = 1 << 31; i > 0; i = i / 2) {
        if(n & i) strcat(test, "1");
        else strcat(test, "0");
        if(c % 8 == 0 && c < 32) strcat(test, ".");
        c++;
    }
    return strdup(test);
}

/* display an error message, if 2nd parameter is true, shut down */
void error_exit(char *errorMessage, bool shutdown) {
    printf("%s\n", errorMessage);
    if(shutdown) exit(1);
}

void catch_function(int sig) {    
    if(sig == SIGINT) {
        error_exit("\n[kern] Prozess unerwartet beendet.", false);
    }
    if(sig == SIGABRT) {
        error_exit("\n[kern] VM zeigt abnormales Verhalten!\n", false);
    }
    if(sig == SIGILL) {
        error_exit("\n[kern] Illegaler Hardwarebefehl!\n", false);
    }
    if(sig == SIGSEGV) {
        error_exit("\n[kern] Segmentfehler! VM wird beendet.\n", true);
    }
    if(sig == SIGTERM) {
        error_exit("\n[kern] VM wird unerwartet beendet.\n", true);
    }
    return;
}

