#include "parser.h"
#include <cstdio>
#include <string.h>

int findLastIndex(char *str, const char x) { 

    int index = -1; 
    for (int i = 0; i < strlen(str); i++) { 
        if (str[i] == x) 
            index = i; 
    } 
    return index; 
    
} 

int main(int argc, char ** argv) {

    Parser parser;
    
    //parser.setDebug(true);
    
    printf("VM Assembler v1.0 (https://github.com/zarat/vm)\n");
    
    if(argc == 3) {
        
        printf("Parsing '%s', write output to '%s'\n", argv[1], argv[2]);
        parser.parseFile(argv[1], argv[2]);
    
    } else if(argc == 2) {
        
        char *filename = argv[1];
        char newfilename[128] = {0};
        int index = findLastIndex(filename, '.');
        sprintf(newfilename, "%.*s.zvm", index, filename);
        printf("Parsing '%s', write output to '%s'\n", argv[1], newfilename);
        parser.parseFile(argv[1], newfilename);
    
    } else {
        
        printf("Usage: %s <assembly|infile> <executable|outfile>", argv[0]);
    
    }
    
    return 0;
    
}
