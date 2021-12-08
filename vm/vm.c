/*

MIT License

Copyright (c) 2018 Manuel Zarat

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and 
associated documentation files (the "Software"), to deal in the Software without restriction, including 
without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell 
copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to 
the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial 
portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT 
LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION 
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

/* INCLUDES */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>

#ifdef _WIN32
#include <conio.h> 	
#else
#include <termios.h>
#endif

/* DEFINES */
#define STACK_SIZE 1024
#define NUM_REG 14

/* GLOBALS */
unsigned int *program;
int regs[NUM_REG + 1]; 

int instrNum, reg1, reg2, reg3, value = 0; 

int stack[STACK_SIZE] = {0};
int pstack = 0;
 
int returnstack[STACK_SIZE] = {0};
int rstack = 0;

bool debug = false; 
bool realtime = false;
bool zeroflag = false;

int pc = 0;
int running = 0;
int displayMode = 0; 

bool bootfilewriteable = false;

enum {
    MEMORY_RW_CHAR = 1,
    MEMORY_RW_INT = 2
};

int memory_rw_mode = MEMORY_RW_CHAR;

enum {
    ARITH_CHAR = 1,
    ARITH_INT = 2,
    ARITH_FLOAT = 3,
};

int arith_mode = ARITH_CHAR;

typedef struct {
    char* bootfile;
    bool writeable;
    bool debug;   
} Configuration;

Configuration config;

static int handler(void* userConfig, const char* section, const char* name, const char* value) {

    Configuration* pconfig = (Configuration*)userConfig;

    #define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
    
    if (MATCH("general", "bootfile")) {
        pconfig->bootfile = strdup(value);
    } 
    else if (MATCH("general", "writeable")) {
        pconfig->writeable = strcmp(value, "true") == 0 ? true : false;
    } 
    else if (MATCH("general", "debug")) {
        pconfig->debug = strcmp(value, "true") == 0 ? true : false;
    }
    else {
        return 0;
    }
    
    return 1;
    
}

#include "Common.h"
#include "Memory.h"
#include "Storage.h"
#include "ini.h"

/* push/pop the variable stack */
void push(int v) {
    if(pstack >= STACK_SIZE) { 
        char dbg[] = "";
        sprintf(dbg, "\n[!!!!!] Stack overflow! pc: %d\n", pc);
        error_exit(dbg, true);  
    }
    stack[pstack] = v;
    pstack++;     
}
int popv() {
    if(pstack <= 0) { 
        char dbg[] = "";
        sprintf(dbg, "\n[!!!!!] Stack underflow! pc: %d\n", pc);
        error_exit(dbg, true);  
    }
    pstack--;
	return stack[pstack];
} 

/* return stack */
void rpush(int v) {
    returnstack[rstack++] = v;	
    if(rstack == STACK_SIZE) { 
        char dbg[] = "";
        sprintf(dbg, "\n[!!!!!] Returnstack overflow! pc: %d rs: %d\n", pc, returnstack[rstack]);
        error_exit(dbg, true);
    }       
}
int rpopv() {
    if(rstack <= 0) {
        char dbg[] = "";
        sprintf(dbg, "\n[!!!!!] Returnstack underflow! pc: %d rs: %d\n", pc, returnstack[rstack]);
        error_exit(dbg, true);
    }
    rstack--;       
    return returnstack[rstack];
}

void decode(int instr) {
	instrNum   = (instr & 0xFF000000) >> 24;
	reg1       = (instr & 0x00FF0000) >> 16;
	reg2       = (instr & 0x0000FF00) >> 8;
	reg3       = (instr & 0x0000FFFF);     // @deprecated
}

void fetch(int *instr) {
	*instr = program[pc];
	value = program[pc + 1];
	pc += 2;
}

void regDump() {
    printf("\nREGISTERS # # # # # # # # # # # # # # # # # # # # # # # # #\n");
    
    //printf("AX: 0x%08x,\tBX: 0x%08x\tCX: 0x%08x,\tDX: 0x%08x\n", regs[1], regs[2], regs[3], regs[4]);
    printf("AX:\t%s 0x%08x %d", int2bin(regs[1]), regs[1], regs[1]);
    printf("\nBX:\t%s 0x%08x %d", int2bin(regs[2]), regs[2], regs[2]);
    printf("\nCX:\t%s 0x%08x %d", int2bin(regs[3]), regs[3], regs[3]);
    printf("\nDX:\t%s 0x%08x %d", int2bin(regs[4]), regs[4], regs[4]);
    
    printf("\n");
    
    //printf("R1: 0x%08x, R2: 0x%08x, R3: 0x%08x\nR4: 0x%08x, R5: 0x%08x, R6: 0x%08x\n", regs[5], regs[6], regs[7], regs[8], regs[9], regs[10]);
    printf("\nR1:\t%s 0x%08x %d", int2bin(regs[5]), regs[5], regs[5]);
    printf("\nR2:\t%s 0x%08x %d", int2bin(regs[6]), regs[6], regs[6]);
    printf("\nR3:\t%s 0x%08x %d", int2bin(regs[7]), regs[7], regs[7]);
    printf("\nR4:\t%s 0x%08x %d", int2bin(regs[8]), regs[8], regs[8]);
    printf("\nR5:\t%s 0x%08x %d", int2bin(regs[9]), regs[9], regs[9]);
    
    printf("\n");
    
    printf("\nR6:\t%s 0x%08x %d", int2bin(regs[10]), regs[10], regs[10]);    
    printf("\nR7:\t%s 0x%08x %d", int2bin(regs[11]), regs[11], regs[11]);
    printf("\nR8:\t%s 0x%08x %d", int2bin(regs[12]), regs[12], regs[12]);    
    printf("\nR9:\t%s 0x%08x %d", int2bin(regs[13]), regs[13], regs[13]);
    printf("\nR10:\t%s 0x%08x %d", int2bin(regs[14]), regs[14], regs[14]);
    
    printf("\n\nzeroflag: %d", (zeroflag) ? 1:0);
    
    printf("\n# # # # # # # # # # # # # # # # # # # # # # # # # # # # # #\n\n");
}

void stackDump() {
    int i = pstack;
    int col = 0;
    printf("\nSTACK DUMP # # # # # # # # # # # # # # # # # # # # # # # #\n");
    char *pattern;
    switch(displayMode) {
        case 0: pattern = "0x%08x "; break;
        case 1: pattern = "%d "; break;
        case 2: pattern = "%c "; break;
        default: pattern = "0x%08x "; break;
    }
    if(i > 0) {
        printf(pattern, stack[--i]);     
        printf(" <-- top\n");
    }
    while(i > 0) {
        printf(pattern, stack[--i]); //col++;
        //if(col == 16) { printf("\n"); col = 0; } 
        //if(i==0) 
        printf("\n");           
    }
    printf("TOTAL: %d elements / %d bytes", pstack, pstack*4 );
    printf("\n# # # # # # # # # # # # # # # # # # # # # # # # # # # # # #\n\n"); 
}

/* dump the memory */
void memDump() {
    struct node *ptr = head;
    printf("\nMEMORY DUMP # # # # # # # # # # # # # # # # # # # # # # # #\n");
    int vSize = 0;
    int count = 0;
    char *pattern;
    switch(displayMode) {
        case 0: pattern = "0x%02x "; break;
        case 1: pattern = "%d ";break;
        case 2: pattern = "%c "; break;
        default: pattern = "0x%02x "; break;
    } 	
    while(ptr != NULL) {
        printf("[ %d ] ", ptr->key );    
        for(int i = 0; i < ptr->len; i++) 
            printf(pattern, ptr->data[i]);
        printf("\n"); vSize += ptr->len; ptr = ptr->next; count++;
    }	
    printf("TOTAL: %d elements / %d bytes\n", count, vSize );
    printf("# # # # # # # # # # # # # # # # # # # # # # # # # # # # # #\n\n");
    free(ptr);
}

void eval() {

    if(debug) {
        printf("rs: %d, ps %d, pc: %d\t| ins: %d, r1: %d, r2: %d, val: %d\n", rstack, pstack, pc, instrNum, reg1, reg2, value); 
    }
    
	switch(instrNum) {
     
		case END: {
			running = 0;
			break;
		}
		case MOV: {
            /*                        
            Move into register
            parameters can be 2 registers, or a register and an immediate value
            when moving reg to reg, the value of the moved reg still remains
            @todo at/t or intel??
            */            
            if(reg2 == 0) regs[reg1] = value; 
            else regs[reg1] = regs[reg2]; 
			break;            
        }        
		case PUSH: {
			/*
            Push a value on the stack
            argument is a register or an immediate value
            if its a register, it gets cleared, if you dont want to clear the register use ldr
            */
            if(reg1 != 0) {
                push(regs[reg1]);
                regs[reg1] = 0;
            } 
            else push(value);
			break;
		}
        case POP: {
			/*
            Pop a value off the stack
            argument is the register where to store the value
            if no argument is shipped, the value is dropped
            */
            regs[reg1] = popv();
			break;
		}
		case LDR: {
			/*
            Load a register on the stack
            argument is the register
            the value still remains in the register, if you want to clear it use pop
            */
            push(regs[reg1]);
			break;
		}
		case STR: {
			/*
            Store a value from the stack into a register
            the value remains on the stack, if you want to clear it use push
            */
            
            /*
            int i = popv();
            regs[reg1] = i;
            push(i);
            */
            
            unsigned char *tmp = (unsigned char *)malloc(4);
            
            memcpy(tmp, &stack[--pstack], 4);
            
            pstack++;

            memcpy(&regs[reg1], tmp, 4);

			break;
		}
        case LDM: {
            /*
            Load data from memory location:position onto the stack
            
            push loc
            push pos
            ldm
            
            @fix 4.12.21 - added switch to read char or int data
            */
            
            if(memory_rw_mode == MEMORY_RW_CHAR)  {
                       
                int loc, pos;
                pos = popv();
                loc = popv();
                
                struct node *foundLink = find(loc);
                                
                int c = (int)foundLink->data[pos];
                
                push(c);
                
            }
            
            else if(memory_rw_mode == MEMORY_RW_INT) {
            
                int loc, pos;
                pos = popv();
                loc = popv();
                
                struct node *foundLink = find(loc);
                                
                int i;
                memcpy(&i, &foundLink->data[pos], sizeof(int)); 
                
                push(i);
            
            }
            
            break;
        }
        case STM: {
            /*
            Store data at a memory location:position
            The location has to be already initialized using puts!
                         
            push value
            push loc
            push pos
            stm
            
            @todo - switch to write int or char data
                    4.12.21 - testing!
            */
            
            if(memory_rw_mode == MEMORY_RW_CHAR) {
            
                int val, loc, pos;
                pos = popv();
                loc = popv();
                val = popv(); 
                
                // lookup existing data, we assume there is data               
                struct node *foundLink = find(loc);             
                
                // if the new position is higher than the current highest index we have to reallocate memory            
                if(pos >= foundLink->len) {
                
                    // calculate the difference
                    int diff = pos - foundLink->len;
                    
                    // create temporary buffer
                    char *tmp = (char*)malloc(foundLink->len + diff);
                    // zero out to be safe
                    memset(tmp, 0, foundLink->len + diff);
                    
                    // copy existing data
                    int c = 0;
                    while(c < foundLink->len) {
                        tmp[c] = foundLink->data[c];
                        c++;
                    }
                    
                    // replace the requested poition by the given value
                    tmp[pos] = val; 
                                                    
                    // store it
                    deleteNode(loc);                
                    insertFirst(loc, tmp, foundLink->len + diff + 1);                
                    if(config.bootfile && config.writeable) {
                        deleteFile(config.bootfile, loc);
                        createFile(config.bootfile, loc, tmp, foundLink->len + diff + 1);
                    }
                    
                } else {
                    
                    // create temporary buffer
                    char *tmp = (char *)malloc(foundLink->len);
                    
                    // copy existing data
                    int i = 0;
                    while(i < foundLink->len) {
                        tmp[i] = foundLink->data[i];
                        i++;
                    }
                    
                    // replace new data
                    tmp[pos] = val;
                    
                    // store it
                    deleteNode(loc);                
                    insertFirst(loc, tmp, foundLink->len);
                    if(config.bootfile && config.writeable) {
                        deleteFile(config.bootfile, loc);
                        createFile(config.bootfile, loc, tmp, foundLink->len);
                    } 
                                  
                }
                
            }
            
            else if(memory_rw_mode == MEMORY_RW_INT) {
            
                int val, loc, pos;
                pos = popv();
                loc = popv();
                val = popv(); 
                               
                struct node *foundLink = find(loc);             
                
                // if the new position is higher than the current highest index we have to reallocate memory            
                if(pos + sizeof(int) >= foundLink->len) {
                
                    // get the difference
                    int diff = (pos + sizeof(int)) - foundLink->len; 
                    
                    // create temporary buffer                   
                    char *tmp = (char*)malloc(foundLink->len + diff);
                    // zero out to be safe
                    memset(tmp, 0, foundLink->len + diff);
                    
                    // copy existing data into buffer
                    int c = 0;
                    while(c < foundLink->len) {
                        tmp[c] = foundLink->data[c];
                        c++;
                    }   
                                     
                    // replace the requested poition by the given value                                                     
                    memcpy(&tmp[pos], &val, sizeof(int));
                    
                    // store it
                    deleteNode(loc);                
                    insertFirst(loc, tmp, foundLink->len + diff);                
                    if(config.bootfile && config.writeable) {
                        deleteFile(config.bootfile, loc);
                        createFile(config.bootfile, loc, tmp, foundLink->len + diff);
                    }   
                                     
                } else {    
                                             
                    // create temporary buffer
                    char *tmp = (char *)malloc(foundLink->len);
                    
                    // copy existing data
                    int i = 0;
                    while(i < foundLink->len) {
                        tmp[i] = foundLink->data[i];
                        i++;
                    }
                    // replace the requested poition by the given value
                    memcpy(tmp + pos, &val, sizeof(int)); 
                    
                    // store it
                    deleteNode(loc);                
                    insertFirst(loc, tmp, foundLink->len);
                    if(config.bootfile && config.writeable) {
                        deleteFile(config.bootfile, loc);
                        createFile(config.bootfile, loc, tmp, foundLink->len);
                    } 
                                                     
                } 
                                
            }
            
            break;
            
        }
        
        case LDMR: {
            /*
            Load a range of bytes from memory location onto the stack            
            push loc
            push start
            push end
            ldmr
            
            @todo int/char data switch
            */
            
            int loc, start, end;
            end = popv();
            start = popv();
            loc = popv();
            struct node *dat = find(loc);           
            /* removed 28.11.21, now its reverse
            while(start <= end) {
                push((int)dat->data[start]);
                start++;    
            }
            */
            while(end >= start) {
                push((int)dat->data[end]);
                end--;    
            } 
                       
            break;
        }    
        case STMR: {
            /*
            Store a range of bytes at a memory location            
            push loc
            push start
            push end
            stmr
            
            @todo int/char data switch
            */
            
            int loc, start, end;
            end = popv();
            start = popv();
            loc = popv();
            struct node *dat = find(loc);                        
            // if the new position is higher than the current highest index we have to reallocate memory            
            int len = dat->len; //strlen(dat->data);
            if(end >= len) {
                int diff = end - len;
                char *cnt;
                cnt = (char*)malloc(len + diff);
                // \x00 characters make trouble reading the length, we avoid it by filling the empty gap with \x20
                memset(cnt, 0, len + diff);
                // copy the current content into the new memory
                int c = 0;
                while( c < len ) {
                    cnt[c] = dat->data[c];
                    c++;
                }
                // copy the new content into the new memory
                while(start <= end) {
                    cnt[start] = popv();
                    start++;    
                }
                // replace temporarily
                deleteNode(loc);                
                insertFirst(loc, cnt, len + diff + 1);                
                // replace in bootfile
                if(config.bootfile && config.writeable) {
                    deleteFile(config.bootfile, loc);
                    createFile(config.bootfile, loc, cnt, len + diff + 1);
                }                                
            } else {                       
                // otherwise we copy the current content into the existing memory
                while(start <= end) {
                    dat->data[start] = popv();
                    start++;    
                }            
                char *tmp = (char *)malloc(dat->len);
                int i = 0;
                while(i < dat->len) {
                    tmp[i] = dat->data[i];
                    i++;
                }
                char *newstr = strdup(tmp);
                // replace temporarily
                newstr[dat->len + 1];
                deleteNode(loc);                
                insertFirst(loc, newstr, dat->len);
                // replace in bootfile
                if(config.bootfile && config.writeable) {
                    deleteFile(config.bootfile, loc);
                    createFile(config.bootfile, loc, newstr, dat->len);
                }                         
            }   
                                   
            break;
        }
        case ADD: {
            /*
            Addition
            
            @todo 06.12.21 arith mode
            */
            
            // default
            if(arith_mode == ARITH_CHAR) {
            
                // create a char from dst
                char c1 = (char)regs[reg1];
                
                // create other char either by reg2 or value
                char c2;                
                if(reg2 != 0) 
                    c2 = (char)regs[reg2];
                else 
                    c2 = (char)value;
                    
                // Now we have 2 char values.
                //printf("<c1: %c (%d), c2: %c (%d)>", c1, c1, c2, c2);
                
                // add them together
                c1 += c2;
                
                // and copy as char into reg1
                memcpy(&regs[reg1], &c1, 1);
                
            }
            
            else if(arith_mode == ARITH_INT) {
                
                // create an int from dst
                int i1 = (int)regs[reg1];
                
                // create other int either by reg2 or value
                int i2;                
                if(reg2 != 0) 
                    i2 = (int)regs[reg2];
                else 
                    i2 = (int)value;
                    
                // Now we have 2 int values.
                //printf("<d1: %d, d2: %d>", i1, i2);
                
                // add them together
                i1 += i2;
                
                // and copy as int into reg1
                memcpy(&regs[reg1], &i1, 4);
                 
            }
            
            if(arith_mode == ARITH_FLOAT) {
            
                // create a float from dst
                float f1; // = regs[reg1];
                memcpy(&f1, &regs[reg1], 4);
                
                // create other float either by reg2 or value
                float f2;                
                if(reg2 != 0) { 
                    memcpy(&f2, &regs[reg2], 4); 
                    //f2 = (float)regs[reg2];
                } else { 
                    //memcpy(&f2, &value, 4);
                    f2 = value;
                }
                    
                // Now we have 2 float values.
                //printf("\n<ADD f1: %f, f2: %f>\n", f1, f2);
                
                // div them together
                f1 += f2;
                
                // and copy as float into reg1
                memcpy(&regs[reg1], &f1, 4);
   
            }            
            
            /*
            if(reg2 == 0) 
                regs[reg1] += value; 
            else 
                regs[reg1] += regs[reg2];  
            */
                    
            break;            
		}        		
		case SUB: {
            /*
            Subtraction
            */
            
            // default
            if(arith_mode == ARITH_CHAR) {
            
                // create a char from dst
                char c1 = (char)regs[reg1];
                
                // create other char either by reg2 or value
                char c2;                
                if(reg2 != 0) 
                    c2 = (char)regs[reg2];
                else 
                    c2 = (char)value;
                    
                // Now we have 2 char values.
                //printf("<c1: %c (%d), c2: %c (%d)>", c1, c1, c2, c2);
                
                // sub them together
                c1 -= c2;
                
                // and copy as char into reg1
                memcpy(&regs[reg1], &c1, 1);
                
            }
            
            else if(arith_mode == ARITH_INT) {
                
                // create an int from dst
                int i1 = (int)regs[reg1];
                
                // create other int either by reg2 or value
                int i2;                
                if(reg2 != 0) 
                    i2 = (int)regs[reg2];
                else 
                    i2 = (int)value;
                    
                // Now we have 2 int values.
                //printf("<d1: %d, d2: %d>", i1, i2);
                
                // sub them together
                i1 -= i2;
                
                // and copy as int into reg1
                memcpy(&regs[reg1], &i1, 4);
                 
            }
            
            if(arith_mode == ARITH_FLOAT) {
            
                // create a float from dst
                float f1; // = regs[reg1];
                memcpy(&f1, &regs[reg1], 4);
                
                // create other float either by reg2 or value
                float f2;                
                if(reg2 != 0) { 
                    memcpy(&f2, &regs[reg2], 4); 
                    //f2 = (float)regs[reg2];
                } else { 
                    //memcpy(&f2, &value, 4);
                    f2 = value;
                }
                    
                // Now we have 2 float values.
                //printf("\n<SUB f1: %f, f2: %f>\n", f1, f2);
                
                // div them together
                f1 -= f2;
                
                // and copy as float into reg1
                memcpy(&regs[reg1], &f1, 4);
   
            } 
            
            /*
            if(reg2 != 0) {                            
                regs[reg1] -= regs[reg2];                               
            } else {                            
                regs[reg1] -= value;                                 
            }
            */
            
			break;
		}		
		case MUL: {
            /*
            Multiplikation
            */
            
            // default
            if(arith_mode == ARITH_CHAR) {
            
                // create a char from dst
                char c1 = (char)regs[reg1];
                
                // create other char either by reg2 or value
                char c2;                
                if(reg2 != 0) 
                    c2 = (char)regs[reg2];
                else 
                    c2 = (char)value;
                    
                // Now we have 2 char values.
                //printf("<c1: %c (%d), c2: %c (%d)>", c1, c1, c2, c2);
                
                // mul them together
                c1 *= c2;
                
                // and copy as char into reg1
                memcpy(&regs[reg1], &c1, 1);
                
            }
            
            else if(arith_mode == ARITH_INT) {
                
                // create an int from dst
                int i1 = (int)regs[reg1];
                
                // create other int either by reg2 or value
                int i2;                
                if(reg2 != 0) 
                    i2 = (int)regs[reg2];
                else 
                    i2 = (int)value;
                    
                // Now we have 2 int values.
                //printf("<d1: %d, d2: %d>", i1, i2);
                
                // mul them together
                i1 *= i2;
                
                // and copy as int into reg1
                memcpy(&regs[reg1], &i1, 4);
                 
            }
            
            if(arith_mode == ARITH_FLOAT) {
            
                // create a float from dst
                float f1 = regs[reg1];
                
                // create other float either by reg2 or value
                float f2;                
                if(reg2 != 0) { 
                    f2 = regs[reg2];
                } else { 
                    f2 = value;
                }
                    
                // Now we have 2 float values.
                //printf("\n<MUL f1: %f, f2: %f>\n", f1, f2);
                
                // div them together
                f1 *= f2;
                
                // and copy as float into reg1
                memcpy(&regs[reg1], &f1, 4);
   
            } 
            
            /*
            if(reg2 != 0) {
                regs[reg1] *= regs[reg2]; 
            } else {
                regs[reg1] *= value; 
            } 
            */
            
			break;
		}        
		case DIV: {            
            /*
            Dividision
            */
            
            // default
            if(arith_mode == ARITH_CHAR) {
            
                // create a char from dst
                char c1 = (char)regs[reg1];
                
                // create other char either by reg2 or value
                char c2;                
                if(reg2 != 0) 
                    c2 = (char)regs[reg2];
                else 
                    c2 = (char)value;
                    
                // Now we have 2 char values.
                //printf("<c1: %c (%d), c2: %c (%d)>", c1, c1, c2, c2);
                
                // div them together
                c1 /= c2;
                
                // and copy as char into reg1
                memcpy(&regs[reg1], &c1, 1);
                
            }
            
            else if(arith_mode == ARITH_INT) {
                
                // create an int from dst
                int i1 = (int)regs[reg1];
                
                // create other int either by reg2 or value
                int i2;                
                if(reg2 != 0) 
                    i2 = (int)regs[reg2];
                else 
                    i2 = (int)value;
                    
                // Now we have 2 int values.
                //printf("<d1: %d, d2: %d>", i1, i2);
                
                // div them together
                i1 /= i2;
                
                // and copy as int into reg1
                memcpy(&regs[reg1], &i1, 4);
                 
            }
            
            if(arith_mode == ARITH_FLOAT) {
            
                // create a float from dst
                float f1 = regs[reg1];
                
                // create other float either by reg2 or value
                float f2;                
                if(reg2 != 0) { 
                    f2 = regs[reg2];
                } else { 
                    f2 = value;
                }
                    
                // Now we have 2 float values.
                // printf("\n<DIV f1: %f, f2: %f>\n", f1, f2);
                
                // div them together
                f1 /= f2;
                
                // and copy as float into reg1
                memcpy(&regs[reg1], &f1, 4);
   
            }
            
            /*
            if(reg2 != 0) {            
                regs[reg1] /= regs[reg2];             
            } else {             
                regs[reg1] /= value;             
            } 
            */
                        
			break;            
		}
        case MOD: {
            /*
            Modulo
            there is no modulo in ARITH_FLOAT mode, use INT instead
            */
            
            // default
            if(arith_mode == ARITH_CHAR) {
            
                // create a char from dst
                char c1 = (char)regs[reg1];
                
                // create other char either by reg2 or value
                char c2;                
                if(reg2 != 0) 
                    c2 = (char)regs[reg2];
                else 
                    c2 = (char)value;
                    
                // Now we have 2 char values.
                //printf("<c1: %c (%d), c2: %c (%d)>", c1, c1, c2, c2);
                
                // mod them together
                c1 %= c2;
                
                // and copy as char into reg1
                memcpy(&regs[reg1], &c1, 1);
                
            }
            
            else if(arith_mode == ARITH_INT || arith_mode == ARITH_FLOAT) {
                
                // create an int from dst
                int i1 = (int)regs[reg1];
                
                // create other int either by reg2 or value
                int i2;                
                if(reg2 != 0) 
                    i2 = (int)regs[reg2];
                else 
                    i2 = (int)value;
                    
                // Now we have 2 int values.
                //printf("<d1: %d, d2: %d>", i1, i2);
                
                // mod them together
                i1 %= i2;
                
                // and copy as int into reg1
                memcpy(&regs[reg1], &i1, 4);
                 
            }
            
            /*
            if(reg2 != 0) {
                regs[reg1] %= regs[reg2]; 
            } else {
                regs[reg1] %= value; 
            }
            */
              
			break;
		}		
		case JMP: {
			/* 
            Jump to a label 
            no return address gets stored
            */
            pc = value * 2;
			break;
		}
		case JZ: {
			/* 
            Jump to a label if zeroFlag = true (last condition was true) 
            no return address gets stored
            */
            if(zeroflag) {
				pc = value * 2;             
			} 
            zeroflag = false;
			break;
		}        
        case JNZ: {
            /* 
            Jump to a label if zeroflag = false (last condition was false) 
            no return address gets stored
            */
			if(!zeroflag) {
                pc = value * 2;                                
			} 
            zeroflag = false;
			break;
		}
        case EQ: {
            /*
            push 1
            push 1
            eq --> z = true
            */
            int b = popv();
            int a = popv();
            zeroflag = false;
            if(a == b) zeroflag = true;
            break;
		}                
        case LT: {
            /*
            push 2
            push 1
            lt --> z = true
            */
            int a = popv();
            int b = popv();
            zeroflag = false;
            if(a < b) zeroflag = true;
			break;
		}          
        case GT: {
            /*
            push 1
            push 2
            gt --> z = true
            */
            int a = popv();
            int b = popv();
            zeroflag = false;
            if(a > b) zeroflag = true;
			break;
		}                 
        case LEQ: {
            /*
            push 2
            push 1
            leq --> zero = true
            */
            int a = popv();
            int b = popv();
            zeroflag = false;
            if(a <= b) zeroflag = true;
            break;
		}                
        case GEQ: {
			/*
            push 1
            push 2
            geq --> zero = true
            */
            int a = popv();
            int b = popv();
            zeroflag = false;
            if(a >= b) zeroflag = true;
            break;
		}
        case RET: {
            pc = rpopv();
			break;
		}		
		case PRINT: {
        
            char fmt = popv();
            char s[] = "%";
            sprintf(s, "%s%c", s, fmt);
            if(arith_mode == ARITH_CHAR) {
                char i;
                memcpy(&i, &stack[--pstack], 1);
                printf(s, i);
			}
            else if(arith_mode == ARITH_INT) {
                int i;
                memcpy(&i, &stack[--pstack], 4);
                printf(s, i);
			}
            else if(arith_mode == ARITH_FLOAT) {
                float f;
                memcpy(&f, &stack[--pstack], 4);
                printf(s, f);
			}
            
            break;
		}       
        case PRINTC: {
            printf("%c", (char)popv());
			break;
		}                
        case READ: {
            /*
            Read a line from stdin
            replace \x0A at the end by \0x00
            */ 
                             
            int index = popv();
            char tmp[1024] = {0};
            fgets(tmp, 1024, stdin); 
            tmp[strlen(tmp)-1] = '\0'; // -1 to remove 0xA on end                              
            int dataLen = strlen(tmp);                        
            struct node *foundLink = find(index);
            if(foundLink != NULL) {
                deleteNode(index);
                if( config.bootfile && bootfilewriteable ) {
                    deleteFile(config.bootfile, index);
                }
            } 
            // add \x00 at the end            
            char *newstr = (char *)malloc(dataLen + 1);  
            int i = 0; 
            while(i < dataLen) { 
                newstr[i] = tmp[i]; 
                i++; 
            }
            newstr[i] = '\0'; // \x00 to mark the end       
            insertFirst(index, newstr, dataLen + 1);                    
            if( config.bootfile && bootfilewriteable ) { 
                createFile(config.bootfile, index, newstr, dataLen + 1);                        
            }            
            // push the length onto the stack afterward?
            // push(dataLen); 
            
            break;                
        }       
        case WRITE: {     
            /*
            Print a memory location as characters to screen
            the location is on the stack
            printf with the string formatter stops at \x00 characters so we print each character in a loop
            */ 
            
            int index = popv();
            struct node *foundLink = find(index);
            char *str = foundLink->data;
            str[foundLink->len] = '\0';
            // printf("%s", str); 
            for(int i = 0; i < foundLink->len; i++) 
                printf("%c", str[i]); 
                       
            break;
		}        
        case PUTS: {       
            /*
            Put data from the stack into memory
            all the data on the stack
            data length on the stack
            memory location on the stack
            
            @fix 3.12.21 - added switch to write char or int data            
            @fix 4.12.21 - using memcpy to fix overwriting other entries
            */
            
            if(memory_rw_mode == MEMORY_RW_CHAR) {
            
                int index = popv();
                int len = popv(); 
                                      
                char tmp[len];
                int i = 0;            
                while(i < len) 
                    tmp[i++] = popv();
                                                            
                struct node *foundLink = find(index);
                if(foundLink != NULL) {
                    deleteNode(index);
                    if( config.bootfile && bootfilewriteable ) {
                        deleteFile(config.bootfile, index);
                    }
                }
                
                unsigned char *buffer = (unsigned char *)malloc(len);
                memcpy(buffer, &tmp, len);
                            
                insertFirst(index, buffer, i);                    
                if( config.bootfile && bootfilewriteable ) { 
                    createFile(config.bootfile, index, buffer, len);
                }  
                              
            }
            
            else if(memory_rw_mode == MEMORY_RW_INT) {
            
                int index = popv();
                int len = popv(); 
                                                                  
                int tmp[len];
                int i = 0;            
                while(i < len)
                    tmp[i++] = popv();
                                                                                             
                struct node *foundLink = find(index);
                if(foundLink != NULL) {
                    deleteNode(index);
                    if( config.bootfile && bootfilewriteable ) {
                        deleteFile(config.bootfile, index);
                    }
                } 
                                          
                unsigned char *buffer = (unsigned char *)malloc(len * sizeof(int));
                memcpy(buffer, &tmp, sizeof(int) * len);                
                
                insertFirst(index, buffer, len * 4);
                if( config.bootfile && bootfilewriteable ) { 
                    createFile(config.bootfile, index, (unsigned char *)tmp, len * 4);
                } 
                 
            }
                                       
            break;
        }
        case GETS: {       
            /*
            Get data from memory onto the stack
            memory location is on the stack
            
            @fix 3.13.21 - added switch to read char or int
            */
            
            if(memory_rw_mode == MEMORY_RW_CHAR) {
                
                int index = popv();
                
                struct node *foundLink = find(index);
                int dataLen = foundLink->len;
                
                while(dataLen > 0) { 
                    push(foundLink->data[--dataLen]);                               
                }
                
            }
            
            else if(memory_rw_mode == MEMORY_RW_INT) {
            
                int index = popv();
                
                struct node *foundLink = find(index);
                int dataLen = foundLink->len;
                
                int i = 0;
                while(dataLen > 0) {                     
                    dataLen -= 4;
                    memcpy(&i, &foundLink->data[dataLen], sizeof(int));
                    push(i);                                                   
                }
                
            }
            
            break;
        }       
        case READC: {                  
            /* 
            Read a char from stdin 
            */
            
            int ch;
            #ifdef _WIN32
            ch = getch();
            #else
            ch = getchar();
            #endif
            push(ch);
            
            break;
		}        
        case CMP: {
            int src = popv();
            int dst = popv();
            struct node *first = find(src);
            struct node *second = find(dst);
            zeroflag = false;
            if(first->len != second->len) { 
                zeroflag = false; 
                break; 
            }
            if( memcmp(first->data, second->data, first->len) == 0 ) 
                zeroflag = true; 
            break;           
		}
        case PRC: {
            /*
            Read from a system process and store the output in memory
            location of command on the stack
            dst where to store output on the stack
            
            @todo bad bytes \x00
            @toto dynamic allocation
            */
                                    
            int dst = popv(); 
            int commandString = popv(); 
                        
            struct node *cmd = find(commandString);
                        
            FILE *f;
            #ifdef _WIN32
            f = _popen(cmd->data, "rt");            
            #else
            f = popen(cmd->data, "rt");
            #endif

            char tmp[1024] = {0};
            char output[1024*128] = {0};                   
            while(fgets(tmp, 1024, f) != NULL) 
                strcat(output, tmp);             
            output[strlen(output)] = '\0';            
            int dataLen = strlen(output);
            
            deleteNode(dst);
            if( config.bootfile && bootfilewriteable ) {
                deleteFile(config.bootfile, dst);
            }
           
            insertFirst(dst, output, dataLen );              
            if( config.bootfile && bootfilewriteable ) { 
                createFile(config.bootfile, dst, output, dataLen );
            }                                       

            // @fix - 06.12.21 f**k, close the damn process at the end!
            #ifdef _WIN32
            _pclose(f);
            #else
            pclose(f);
            #endif
            
            break;
                       
		}
        case SI: {
            regs[reg1] = pstack;
            break;
        }
        case INC: {
            if(arith_mode == ARITH_CHAR || arith_mode == ARITH_INT) {
                regs[reg1] += 1;
            }
            else {
                float f; 
                memcpy(&f, &regs[reg1], 4); // = regs[reg1];
                f += 1.0f;
                //printf("<INC f=%f>", f);
                memcpy(&regs[reg1], &f, 4);
            }
            break;
        }
        case DEC: {
            if(arith_mode == ARITH_CHAR || arith_mode == ARITH_INT) {
                regs[reg1] -= 1;
            }
            else {
                float f; 
                memcpy(&f, &regs[reg1], 4); // = regs[reg1];
                f -= 1.0f;
                //printf("<DEC f=%f>", f);
                memcpy(&regs[reg1], &f, 4);
            }
            break;
        }
        case CALL: {
            /* 
            Call a label 
            The return address get stored
            */            
            rpush(pc);
            pc = value * 2;
            break;
        }
        case INT: {
            /* 
            interrupt call
            */
            int r;  
                      
            if(reg1 != 0)                             
                r = regs[reg1];                               
            else                            
                r = value;
                                                 
            switch(r) {
            
                // R/W single bytes to/from memory
                case 1:
                    memory_rw_mode = MEMORY_RW_CHAR;
                    break;
                    
                // R/W 4 bytes at once from/to memory
                case 2:
                    memory_rw_mode = MEMORY_RW_INT;
                    break;
                    
                case 3:
                    stackDump();
                    break;
                case 4:
                    memDump();
                    break;
                case 5:
                    regDump();
                    break;                
                
                case 9:
                    arith_mode = ARITH_CHAR;
                    break;
                case 10:
                    arith_mode = ARITH_INT;
                    break;
                case 11:
                    arith_mode = ARITH_FLOAT;
                    break;
                    
                default:
                    break;  
                      
            }
            
            break;
        }		
		default: {
			printf("[kern] bad instruction '%d' at pc '%d'\n", instrNum, pc);
            break;
        }           
	}
   
}

void run() {
    running = 1;
	int instr;
	while(running) { 
		fetch(&instr);
		decode(instr);
		eval();
	}
    running = false; 
}

void translateOpCode(char *token) {
    if(strcmp(token, "eof") == 0) instrNum = EOF;
    else if(strcmp(token, "mov") == 0) instrNum = MOV;  
    else if(strcmp(token, "push") == 0) instrNum = PUSH; 
    else if(strcmp(token, "pop") == 0) instrNum = POP; 
    else if(strcmp(token, "ldr") == 0) instrNum = LDR; 
    else if(strcmp(token, "str") == 0) instrNum = STR; 
    else if(strcmp(token, "ldm") == 0) instrNum = LDM; 
    else if(strcmp(token, "stm") == 0) instrNum = STM;
    else if(strcmp(token, "ldmr") == 0) instrNum = LDMR; 
    else if(strcmp(token, "stmr") == 0) instrNum = STMR; 
    else if(strcmp(token, "add") == 0) instrNum = ADD; 
    else if(strcmp(token, "addi") == 0) instrNum = ADDI; 
    else if(strcmp(token, "sub") == 0) instrNum = SUB; 
    else if(strcmp(token, "mul") == 0) instrNum = MUL; 
    else if(strcmp(token, "div") == 0) instrNum = DIV;  
    else if(strcmp(token, "mod") == 0) instrNum = MOD; 
    else if(strcmp(token, "eq") == 0) instrNum = EQ; 
    else if(strcmp(token, "lt") == 0) instrNum = LT;  
    else if(strcmp(token, "gt") == 0) instrNum = GT; 
    else if(strcmp(token, "leq") == 0) instrNum = LEQ; 
    else if(strcmp(token, "geq") == 0) instrNum = GEQ; 
    else if(strcmp(token, "jmp") == 0) instrNum = JMP; 
    else if(strcmp(token, "jz") == 0) instrNum = JZ; 
    else if(strcmp(token, "jnz") == 0) instrNum = JNZ; 
    else if(strcmp(token, "ret") == 0) instrNum = RET; 
    else if(strcmp(token, "print") == 0) instrNum = PRINT;  
    else if(strcmp(token, "printc") == 0) instrNum = PRINTC;  
    else if(strcmp(token, "read") == 0) instrNum = READ;  
    else if(strcmp(token, "write") == 0) instrNum = WRITE;  
    else if(strcmp(token, "puts") == 0) instrNum = PUTS; 
    else if(strcmp(token, "gets") == 0) instrNum = GETS; 
    else if(strcmp(token, "readc") == 0) instrNum = READC; 
    else if(strcmp(token, "cmp") == 0) instrNum = CMP; 
    else if(strcmp(token, "prc") == 0) instrNum = PRC;
    else if(strcmp(token, "si") == 0) instrNum = SI;
    else if(strcmp(token, "inc") == 0) instrNum = INC;
    else if(strcmp(token, "dec") == 0) instrNum = DEC;
    else if(strcmp(token, "int") == 0) instrNum = INT;
}

int translateReg1(char *token) {
    if(strcmp(token, "ax") == 0) reg1 = 1; 
    else if(strcmp(token, "bx") == 0) reg1 = 2; 
    else if(strcmp(token, "cx") == 0) reg1 = 3; 
    else if(strcmp(token, "dx") == 0) reg1 = 4; 
    else if(strcmp(token, "r1") == 0) reg1 = 5;
    else if(strcmp(token, "r2") == 0) reg1 = 6;
    else if(strcmp(token, "r3") == 0) reg1 = 7;
    else if(strcmp(token, "r4") == 0) reg1 = 8;
    else if(strcmp(token, "r5") == 0) reg1 = 9;
    else if(strcmp(token, "r6") == 0) reg1 = 10;
    else if(strcmp(token, "r7") == 0) reg1 = 11;
    else if(strcmp(token, "r8") == 0) reg1 = 12;
    else if(strcmp(token, "r9") == 0) reg1 = 13;
    else if(strcmp(token, "r10") == 0) reg1 = 14;
    else {
        value = atoi(token) ? atoi(token) : 0;
        reg1 = 0;
    }
    return reg1;
}

int translateReg2(char *token) {
    if(strcmp(token, "ax") == 0) reg2 = 1; 
    else if(strcmp(token, "bx") == 0) reg2 = 2; 
    else if(strcmp(token, "cx") == 0) reg2 = 3; 
    else if(strcmp(token, "dx") == 0) reg2 = 4; 
    else if(strcmp(token, "r1") == 0) reg2 = 5;
    else if(strcmp(token, "r2") == 0) reg2 = 6;
    else if(strcmp(token, "r3") == 0) reg2 = 7;
    else if(strcmp(token, "r4") == 0) reg2 = 8;
    else if(strcmp(token, "r5") == 0) reg2 = 9;
    else if(strcmp(token, "r6") == 0) reg2 = 10;
    else if(strcmp(token, "r7") == 0) reg2 = 11;
    else if(strcmp(token, "r8") == 0) reg2 = 12;
    else if(strcmp(token, "r9") == 0) reg2 = 13;
    else if(strcmp(token, "r10") == 0) reg2 = 14;
    else {
        value = atoi(token) ? atoi(token) : 0;
        reg2 = 0;
    }
    return reg2;
}

bool storageloaded = false;

void load(char *runnable) {

    FILE * f = fopen(runnable, "rb");
  	if(!f) {
  		fprintf(stderr, "An error occurred while opening the file.\n");
  		exit(EXIT_FAILURE);
  	}		
  	fseek(f, 0, SEEK_END);
  	int len = ftell(f);
  	fseek(f, 0, SEEK_SET);
  	program = malloc(len);
  	fread(program, 1, len, f);
  	fclose(f);
    if(!storageloaded) if( config.bootfile ) readStorage( config.bootfile );
    run();
    instrNum = reg1 = reg2 = value = pc = 0;

}

void realTime() {

    printf("ZVM v1.0 beta (https://github.com/zarat/vm)\n");
        printf("[info] realtime mode enabled, type 'help' for help\n");
        realtime = true;
        char command[1024];
        char *token = NULL;

        if( config.bootfile ) readStorage( config.bootfile );
        storageloaded = true;
        int tokenCounter = 0;        
        while(1) {
            printf("> ");
            fgets(command, 1024, stdin);
            command[strcspn(command, "\n")] = '\0';
            if(startsWith("sta", command)) stackDump();
            if(startsWith("mem", command)) memDump();
            if(startsWith("reg", command)) regDump();
            if(startsWith("quit", command)) return;
            if(startsWith("help", command)) {
                printf("Available commands:\n\tregister\tshow registers\n\tstack\t\tshow stack\n\tmemory\t\tshow memory\n\tclear\t\tclear data\n\tload <file>\tload program\n\tquit\t\texit vm\n");
            }
            if(startsWith("clear", command)) {
                //readStorage();
                memset(regs, 0, sizeof(regs));
                memset(stack, 0, STACK_SIZE);
                pstack = 0;
            }
            if(startsWith("dis", command)) {
                char *token = strtok(command, " ");
                int displayModeCounter = 0;                                                                                        
                while(token != NULL) {
                    if(displayModeCounter==1) {                         
                        token[strcspn(token, "\n")] = '\0';
                        displayMode = atoi(token); break; 
                    }
                    displayModeCounter++;
                    token = strtok(NULL, " ");                    
                }
                token = NULL;
            }
            if(startsWith("exec", command)) {
                char *token = strtok(command, " ");
                int displayModeCounter = 0;                                                                                        
                while(token != NULL) {
                    if(displayModeCounter==1) {                         
                        token[strcspn(token, "\n")] = '\0';
                        eval(token);
                    }
                    token = strtok(NULL, " ");                    
                }
                token = NULL;
            }
            if(startsWith("load", command)) {
                char *token = strtok(command, " ");
                int c = 0;
                char *code;
                while(token != NULL) {
                    if(c == 1)
                        code = token;
                    token = strtok(NULL, " ");
                    c++;                                                                                      
                }                
                token = NULL;
                
                load(code);
                
            }
            
            token = strtok(command, " ");
            while(token != NULL) {
                token[strcspn(token, "\n")] = '\0';
                switch(tokenCounter) {
                    case 0: translateOpCode(token); break;                        
                    case 1: translateReg1(token); break; 
                    case 2: translateReg2(token); break; 
                    case 3: value = atoi(token) ? atoi(token) : 0; break;
                    default: break;
                }
                tokenCounter++;
                token = strtok(NULL, " ");
            }
            token = NULL;
            tokenCounter = 0;
            eval();
            instrNum = reg1 = reg2 = value = 0;            
        }

}

int main(int argc, char ** argv) {     

    config.bootfile = 0;
    config.debug = false;
    
    ini_parse("vm.ini", handler, &config);
    
    debug = config.debug;
    bootfilewriteable = config.writeable;
    
    signal(SIGINT, catch_function);
    signal(SIGABRT, catch_function);
    signal(SIGFPE, catch_function);
    signal(SIGILL, catch_function);
    signal(SIGSEGV, catch_function);
    signal(SIGTERM, catch_function);
    
    displayMode = 0;

    char runnable[64] = {0};
    bool runnableset = false;
    int a = 1;
    
    while(a < argc) {
         
        if(argv[a][0] == '-') {
        
            if(argv[a][1] == 'd')  
                debug = true;
            
            // write memory to mounted bootfile 
            if(argv[a][1] == 'w')  
                bootfilewriteable = true;
             
        } 
        
        else {
        
            runnableset = true;
            strcpy(runnable, argv[a]);
            
        }
        
        a++; 
                
    }

    if(runnableset) {
     
        FILE * f = fopen(runnable, "rb");
    	if(!f) {
    		fprintf(stderr, "An error occurred while opening the file.\n");
    		exit(EXIT_FAILURE);
    	}		
    	fseek(f, 0, SEEK_END);
    	int len = ftell(f);
    	fseek(f, 0, SEEK_SET);
    	program = malloc(len);
    	fread(program, 1, len, f);
    	fclose(f);
        if( config.bootfile ) 
            readStorage( config.bootfile );
        storageloaded = true;
        run(); 
        
    } else { 
       
        realTime();
        
    }
    
    return 0;
    
}