#include "lexer.h"
#include <cstdlib>

Lexer::Lexer(const std::string & fname) {
    f = fopen(fname.c_str(), "r");
    if(!f) {
        fprintf(stderr, "error opening input file '%s'\n", fname.c_str());
        exit(EXIT_FAILURE);
    }   
    last = readChar();
}

Lexer::~Lexer() {
    fclose(f);
}

int Lexer::readChar() {
    alreadyread++;
    return fgetc(f);
}

int Lexer::peek() {
    int i = fgetc(f);
    ungetc(i, f);
    return i;
}

int Lexer::getToken() {

    // whitespaces
	while(last == ' ' || last == '\t') {
		last = readChar();
	}
    // skip comments
	if(last == ';') {
		do { last = readChar();
		} while(last != '\n' && last != EOF);
	}	
	if(last == '\n') {
		last = readChar();
		return EOL;
	}	
	if(last == EOF) {
		return EOF;
	}
    // labels	
	if(last == ':') {
        last = readChar();
        return COLON;
    }
    
    if(last == '\'') {
        last = (char)readChar(); // get the char
        //last -= 87;
        lastInteger = last;
        readChar(); // closing '''
        last = readChar();
        return INTEGER;
    }
    
    if(last == '"') {
        // read a string
        lastString = "";
        last = readChar();
        while(last != '"') {  
            lastString += last;
            last = readChar();
        } 
        last = readChar(); // final "
        //last = readChar();
        return STRING;
    }
	
	// read instructions while isalpha
	if(isalpha(last)) {
    
		lastIdentifier = (char)last;
        while( isalnum(last = readChar()) || last == '_' ) lastIdentifier += last; // for labels		
		if(lastIdentifier == "ax") return AX;
		if(lastIdentifier == "bx") return BX;
		if(lastIdentifier == "cx") return CX;
		if(lastIdentifier == "dx") return DX;
		if(lastIdentifier == "r1") return R1;
        if(lastIdentifier == "r2") return R2;
        if(lastIdentifier == "r3") return R3;
        if(lastIdentifier == "r4") return R4;
        if(lastIdentifier == "r5") return R5;
        if(lastIdentifier == "r6") return R6;
        if(lastIdentifier == "r7") return R7;
        if(lastIdentifier == "r8") return R8;
        if(lastIdentifier == "r9") return R9;
        if(lastIdentifier == "r10") return R10;		
		if(lastIdentifier == "mov") return MOV;
		if(lastIdentifier == "push") return PUSH;
        if(lastIdentifier == "pop") return POP;
		if(lastIdentifier == "ldr") return LDR;
		if(lastIdentifier == "str") return STR;
        if(lastIdentifier == "ldm") return LDM;
		if(lastIdentifier == "stm") return STM; 
        if(lastIdentifier == "ldmr") return LDMR;
		if(lastIdentifier == "stmr") return STMR;       
		if(lastIdentifier == "add") return ADD;
        if(lastIdentifier == "addi") return ADDI;
		if(lastIdentifier == "sub") return SUB;
		if(lastIdentifier == "mul") return MUL;
        if(lastIdentifier == "div") return DIV;
        if(lastIdentifier == "mod") return MOD;        
		if(lastIdentifier == "jmp") return JMP;
		if(lastIdentifier == "jz") return JZ;
        if(lastIdentifier == "jnz") return JNZ;        
		if(lastIdentifier == "print") return PRINT;
        if(lastIdentifier == "printc") return PRINTC;        
        if(lastIdentifier == "read") return READ;
        if(lastIdentifier == "write") return WRITE;        
        if(lastIdentifier == "ret") return RET;
        if(lastIdentifier == "eq") return EQ;
        if(lastIdentifier == "lt") return LT;
        if(lastIdentifier == "gt") return GT;
        if(lastIdentifier == "leq") return LEQ;
        if(lastIdentifier == "geq") return GEQ;
        if(lastIdentifier == "puts") return PUTS;
        if(lastIdentifier == "gets") return GETS;
        if(lastIdentifier == "readc") return READC;
        if(lastIdentifier == "cmp") return CMP;
        if(lastIdentifier == "prc") return PRC;  
        if(lastIdentifier == "si") return SI; 
        if(lastIdentifier == "inc") return INC;  
        if(lastIdentifier == "dec") return DEC; 
        if(lastIdentifier == "call") return CALL;
        if(lastIdentifier == "int") return INT;  
		return LABEL;
	}
    
    if(isdigit(last) || last == '-') { 
        if(last == '-') {  
            int p = peek();
            last = readChar(); // eat it            
            lastInteger = last - '0';            
            while(true) {        
                if( (last = readChar()) == '_' ) continue;                        
                if( isdigit(last) ) {
                    lastInteger *= 10;
                    lastInteger += last - '0';   
                }            
                else break;            
            }        
            lastInteger *= -1;       
            return INTEGER;            
        } else {        
            
            int p = peek();
            
            if(p == 'x') {         
                //printf("peek = %c\n", p);
                char number[32] = {0};
                last = readChar(); // x                
                last = readChar();
                int cc = 0;                
                while( (last>=48 && last<=57) || (last>=97 && last<=102) || (last>=65 && last<=70) ) {                                                                               
                      // digit
                          if((last>=48 && last<=57)) {
                            //printf("digit(%c) ", last);
                            number[cc++] = (char)last;
                          } 
                          // character
                          if((last>=97 && last<=102) || (last>=65 && last<=70) ) {
                            //printf("char(%c) ", last);
                            number[cc++] = last;
                          }                                                                               
                    last = readChar(); 
                }   
                int num = (int)strtol(number, NULL, 16);
                //printf("num: %d\n", num);
                lastInteger = num;
                return INTEGER;                   
            }
            
            else if(p == 'b') {         
                //printf("peek = %c\n", peek);
                char number[8] = {0};                
                last = readChar(); 
                int cc = 0;                
                while( (last>=48 && last<=57) || (last>=97 && last<=102) || (last>=65 && last<=70) ) {                                                                               
                      // digit
                          if((last>=48 && last<=57)) {
                            //printf("digit(%c) ", last);
                            number[cc++] = (char)last;
                          }                                                                             
                    last = readChar(); 
                }   
                int num = (int)strtol(number, NULL, 2);
                //printf("num: %d\n", num);
                lastInteger = num;
                return INTEGER;                   
            }  
            
            lastInteger = last - '0';            
            while(true) {        
                if( (last = readChar()) == '_' ) continue;                        
                if( isdigit(last) ) {
                    lastInteger *= 10;
                    lastInteger += last - '0';
                }            
                else break;            
            } 
            //printf("lastinteger: %d, peek was '%c'\n", lastInteger, p);       
            return INTEGER;            
        } 
               
    }
    
	return EOF; 
}
