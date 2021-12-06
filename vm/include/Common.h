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

/* fucking ugly */
#define BYTE2BIN(byte) \
  (byte & 536870912 ? '1' : '0'), \
  (byte & 268435456 ? '1' : '0'), \
  (byte & 134217728 ? '1' : '0'), \
  (byte & 67108864 ? '1' : '0'), \
  (byte & 33554432 ? '1' : '0'), \
  (byte & 16777216 ? '1' : '0'), \
  (byte & 8388608 ? '1' : '0'), \
  (byte & 4194304 ? '1' : '0'), \
  (byte & 2097152 ? '1' : '0'), \
  (byte & 1048576 ? '1' : '0'), \
  (byte & 524288 ? '1' : '0'), \
  (byte & 65536 ? '1' : '0'), \
  (byte & 262144 ? '1' : '0'), \
  (byte & 65536 ? '1' : '0'), \
  (byte & 131072 ? '1' : '0'), \
  (byte & 65536 ? '1' : '0'), \
  (byte & 32768 ? '1' : '0'), \
  (byte & 16384 ? '1' : '0'), \
  (byte & 8192 ? '1' : '0'), \
  (byte & 4096 ? '1' : '0'), \
  (byte & 2048 ? '1' : '0'), \
  (byte & 1024 ? '1' : '0'), \
  (byte & 512 ? '1' : '0'), \
  (byte & 256 ? '1' : '0'), \
  (byte & 128 ? '1' : '0'), \
  (byte & 64 ? '1' : '0'), \
  (byte & 32 ? '1' : '0'), \
  (byte & 16 ? '1' : '0'), \
  (byte & 8 ? '1' : '0'), \
  (byte & 4 ? '1' : '0'), \
  (byte & 2 ? '1' : '0'), \
  (byte & 1 ? '1' : '0')

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

/* check if a given string starts with a given substring */
bool startsWith(const char *key, const char *str) {
    size_t lenKey = strlen(key), lenStr = strlen(str);
    return lenStr < lenKey ? false : memcmp(key, str, lenKey) == 0;
}

/* get a value from ini file, if not set, return false */
char *ini(char *searchKey) {
    
    FILE * fp;
    char *line = NULL;
    size_t len, read = 0;
    fp = fopen("vm.ini", "r");
    if (fp == NULL) return false;
    char *token = NULL;
    int tokenCounter = 0;
    char *key, *value;
    
    while ((read = getline(&line, &len, fp)) != -1) {
               
        //if(read < 1) continue;
        if(!startsWith(";", line)) {
        
            token = strtok(line, "=");
            
            while(token != NULL) {
            
                if(tokenCounter == 0) { 
                
                    key = token; 
                    key[strcspn(key, "\n")] = '\0'; 
                    
                }
                
                if(tokenCounter == 1) {
                
                    value = token; 
                    value[strcspn(value, "\n")] = '\0';
                    if(strcmp(searchKey, key) == 0) { 
                        return strdup(value);
                    }
                    
                    tokenCounter = 0; // reset after each line!
                        
                }
                
                tokenCounter++;
                token = strtok(NULL, "=");
                
            }
        }
        
        token = NULL;
    }

    fclose(fp);
    if (line) free(line);
    return NULL;
      
}