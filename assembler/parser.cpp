/*
This class parses the AST tree, translates the instructions and save it as a bytestream
*/
#include "parser.h"
#include "lexer.h"
#include <cstdlib>
#include <vector>

/*
Register opcodes have higher numbers than all instructions
so we just have to check is it higher than the highest opcode but lower than <allinstructions> + <internals>
*/
static bool isRegister(int val) {
    return val >= AX && val <= R10;
}

/*
AX is the first known register
substract it from the opcode
e.g:
    AX                          = 14 (OPCODE)
    val                         = 99
    99 - 14 + 1                 = 86
    86 + 13                     = 99 (13 = ALL KNOWN OPCODES)
*/
static int registerValue(int val) {
	return val - AX + 1;
}

struct Label {
    std::string name;
    int pos;
    Label() : pos(-1) {}
    Label(std::string name, int pos) : name(name), pos(pos) {}
};

struct Opcode {
    int instr;
    int value;
    Opcode() : instr(0), value(0) {}
    Opcode(int i, int v) : instr(i), value(v) {}
};

struct Labels {
    std::vector<Label> labels;
    std::vector<Label> unknown;
    int findLabel(const std::string & str) {    
        for(unsigned int i=0; i<labels.size(); ++i) {
            if(labels[i].name == str) return i;
        }
        return -1;        
    }
};

void Parser::setDebug(bool dbg) {
    debug = dbg;
}

void Parser::parseFile(const std::string & fname, const std::string & out) {

    Lexer lex(fname);
    
    Labels labels;
    std::vector<Opcode> instructions;
    
	int instr=0, value=0;
	int cur; // the current read value
    int curLineCount = 1; // human readable
    int curLinePos = 0;

	while((cur = lex.getToken()) != EOF) {
 
        curLinePos++;
        
        if(cur == EOL) { curLineCount++; curLinePos = 0; continue; }

		if(cur == MOV) {                			
            instr = MOV << 24;                        
			cur = lex.getToken();             
			if(isRegister(cur)) instr |= registerValue(cur) << 16; 
			else {                 
                fprintf(stderr, "Register expected as first argument to mov. (line:%d pos:%d)\n", curLineCount, curLinePos);
                exit(EXIT_FAILURE); 
            }            
			cur = lex.getToken();
                        
			if(isRegister(cur)) instr |= registerValue(cur) << 8;  
			else if(cur == INTEGER) value = lex.lastInteger;
            
			else {
                fprintf(stderr, "Integer or Register expected as second argument to mov (line:%d pos:%d)\n", curLineCount, curLinePos);
                exit(EXIT_FAILURE);
			}                        
		}
        
        if( cur == EQ || cur == LT || cur == GT || cur == LEQ || cur == GEQ ) {			
            instr = cur << 24;            
		}

        if(cur == PUSH) { 
               
            instr = PUSH << 24;            
            cur = lex.getToken();            
            if(cur == INTEGER) value = lex.lastInteger;
            else if(isRegister(cur)) instr |= registerValue(cur) << 16;

            else if(cur == STRING) {
                
                int i = lex.lastString.length();
                int j = 0;
                while(i >= 0) {
                    value = lex.lastString[i];
                    //printf("push %c\n", value);
                    if(j > 0)
                        instructions.push_back(Opcode(instr, value));
                    i--;
                    j++;
                }
                lex.lastString = "";
                instr = cur = value = 0;
                continue;
                
            }

            else {
                fprintf(stderr, "Integer or Register rexpected to push (line:%d pos:%d)\n", curLineCount, curLinePos);
                exit(EXIT_FAILURE);
			}
            
            
                        
		}
        
        if(cur == POP) {        
            instr = POP << 24;                        
            cur = lex.getToken();                        
            if(isRegister(cur)) instr |= registerValue(cur) << 16;
			/* its allowed to write only 'pop' then it just gets dropped
            else {
                fprintf(stderr, "Integer or Register expected to pop (line:%d pos:%d)\n", curLineCount, curLinePos);
                exit(EXIT_FAILURE);
			}
            */                        
		}
		
        if(cur == LDR || cur == STR) {
            instr = cur << 24;
            value = 0;
            cur = lex.getToken();
            if(isRegister(cur)) instr |= registerValue(cur) << 16;
            //else if(cur == INTEGER) value = lex.lastInteger;
			else {
                fprintf(stderr, "Register expected to ldr/str (line:%d pos:%d)\n", curLineCount, curLinePos);
				exit(EXIT_FAILURE);
			}
		}
        
        if(cur == LDM || cur == STM) {
            instr = cur << 24;                        
			cur = lex.getToken();
            if(isRegister(cur)) instr |= registerValue(cur) << 16;                                    
			else if(cur == INTEGER) value = lex.lastInteger;
            /* 27.12.19: can also be alone? */
		}
        
        if(cur == ADD || cur == SUB || cur == MUL || cur == DIV || cur == MOD) {        
            instr = cur << 24; 
            //value = 0;                       
			cur = lex.getToken();             
			if(isRegister(cur)) instr |= registerValue(cur) << 16; 
			else {                 
                fprintf(stderr, "Register expected as first argument to add/sub/mul/div/mod (line:%d pos:%d)\n", curLineCount, curLinePos);
                exit(EXIT_FAILURE); 
            }            
			cur = lex.getToken();            
			if(isRegister(cur)) instr |= registerValue(cur) << 8;  
			else if(cur == INTEGER) value = lex.lastInteger;
			else {
                fprintf(stderr, "Integer or Register expected as second argument to add/sub/mul/div/mod (line:%d pos:%d)\n", curLineCount, curLinePos);
                exit(EXIT_FAILURE);
			}         
        }
        
        /* one word instructions */
		if(cur == PRINT || cur == PRINTC || cur == RET || cur == PUTS || cur == GETS || cur == READ || cur == READC || cur == WRITE || cur == CMP || cur == PRC || cur == LDMR || cur == STMR ) {      
            instr = cur << 24;
            value = 0;
		}
        
		if(cur == JZ || cur == JNZ || cur == JMP) {        
            instr = cur << 24;
            if(lex.getToken() == LABEL) {
                int p = labels.findLabel(lex.lastIdentifier);
                if(p != -1) {
                    value = labels.labels[p].pos;
                } else {
                    value = 0xFFFFFFFF;
                    labels.unknown.push_back(Label(lex.lastIdentifier, instructions.size()));
                }
            } else {
                fprintf(stderr, "Label expected to jmp/jz/jnz (line:%d pos:%d)\n", curLineCount, curLinePos);
                exit(EXIT_FAILURE);
            }
		}
        
        if(cur == SI) {        
            instr = cur << 24;            
            cur = lex.getToken();            
            if(isRegister(cur)) instr |= registerValue(cur) << 16;
			else {
                fprintf(stderr, "Register rexpected to si (line:%d pos:%d)\n", curLineCount, curLinePos);
                exit(EXIT_FAILURE);
			}            
		}
		
        if(cur == LABEL) {
            labels.labels.push_back(Label(lex.lastIdentifier, instructions.size()));
            if(lex.getToken() != COLON) {
                fprintf(stderr, "missing ':' after label definition (line:%d pos:%d)\n", curLineCount, curLinePos);
                exit(EXIT_FAILURE);
            }
		}
        
        if(cur == INC) {        
            instr = INC << 24;            
            cur = lex.getToken();            
            if(isRegister(cur)) instr |= registerValue(cur) << 16;
			else {
                fprintf(stderr, "Register rexpected to inc (line:%d pos:%d)\n", curLineCount, curLinePos);
                exit(EXIT_FAILURE);
			}            
		}
        
        if(cur == DEC) {        
            instr = DEC << 24;            
            cur = lex.getToken();            
            if(isRegister(cur)) instr |= registerValue(cur) << 16;
			else {
                fprintf(stderr, "Register rexpected to dec (line:%d pos:%d)\n", curLineCount, curLinePos);
                exit(EXIT_FAILURE);
			}            
		}
        
        if(cur == CALL) {        

            instr = CALL << 24;                        
            //cur = lex.getToken();                        
            if(lex.getToken() == LABEL) {
                int p = labels.findLabel(lex.lastIdentifier);
                if(p != -1) {
                    value = labels.labels[p].pos;
                } else {
                    value = 0xFFFFFFFF;
                    labels.unknown.push_back(Label(lex.lastIdentifier, instructions.size()));
                }
            }
            //if(isRegister(cur)) instr |= registerValue(cur) << 16;
			
            else {
                fprintf(stderr, "LABEL expected to call (line:%d pos:%d)\n", curLineCount, curLinePos);
                exit(EXIT_FAILURE);
			}
                                    
		}
        
        if(cur == INT) { 
               
            instr = INT << 24;            
            cur = lex.getToken();  
            
            if(isRegister(cur)) instr |= registerValue(cur) << 16;          
            else if(cur == INTEGER) value = lex.lastInteger;
			else {
                fprintf(stderr, "Integer or register expected to int (line:%d pos:%d)\n", curLineCount, curLinePos);
                exit(EXIT_FAILURE);
			}            
		}
		
		if(cur != LABEL) 
            instructions.push_back(Opcode(instr, value));

        // clear after each loop
        instr = cur = value = 0;
        
        
        
	}
	
    /* important */
	instructions.push_back(Opcode(0, 0)); // the end
	
	// Für jede Sprunganweisung, bei der das Label noch nicht bekannt war, weisen wir ihm das richtige Label zu
	for(unsigned int i=0; i<labels.unknown.size(); ++i) {
    
        const Label& label = labels.unknown[i];
        int p = labels.findLabel(label.name);
        
        if(p != -1) {
        
            instructions[label.pos].value = labels.labels[p].pos;
            
        } else {
        
            fprintf(stderr, "Label %s not found\n", label.name.c_str());
            exit(EXIT_FAILURE);
            
        }
        
	}

	FILE * f = fopen(out.c_str(), "wb");
    
    if(debug) {
    
        printf("Functions (%d):\n", labels.labels.size());
        
        for(unsigned int i=0; i<labels.labels.size(); ++i) {
        
                printf("0x%08x\t%s\n", labels.labels[i].pos, labels.labels[i].name.c_str() );
                
        }
        
        printf("Code (%d byte):\n", instructions.size() * 8);
        
    }
    
	for(unsigned int i=0; i<instructions.size(); ++i) {
    
        fwrite(&instructions[i].instr, sizeof(int), 1, f);
        
        fwrite(&instructions[i].value, sizeof(int), 1, f);
        
        if(debug) 
            printf("0x%08x\t0x%08X 0x%08X\n", i, instructions[i].instr, instructions[i].value); 
               
	}
  
	fclose(f);
    
}
