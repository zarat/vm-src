/*

A structure of a storage file
Author: Manuel Zarat
Date: 13.12.2019
Copyleft

compile with '-D _DEVEL' to use developer functions

*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* todo: getch() is in none of the ansii headers !!! */
#ifdef _WIN32
#include <conio.h>
#else
#include <termios.h>
#endif

/* the header of a storage file */
struct header {
    int id;
    int len;
};

/* a single entry inside a storage file */
struct entry {
    int id;       
    int len; // length of data        
    char *data;   
};

/* create an empty storage file, id 1, len 0 */
void create() {
    FILE *f = fopen("boot.dat", "wb");   
    // create a header with id=1 and len=2 
    struct header hdr = {1, 0};
    fwrite(&hdr, sizeof(struct header), 1, f);    
    fclose(f);
}

/* create a file */
void createFile(int newId, char *newdata, int dataLen) {
    FILE *f = fopen("boot.dat", "rb");       
    /* get the header */
    struct header inhdr;
    fread(&inhdr.id, sizeof(int), 1, f);
    fread(&inhdr.len, sizeof(int), 1, f);
    /* create a new list to store all existing entries */
    struct entry list[inhdr.len];    
    /* get the entries */
    int id, len = 0; char *data; 
    int inputLength = 0;
    while(inputLength < inhdr.len) {    
        fread(&id, sizeof(int), 1, f);
        fread(&len, sizeof(int), 1, f);
        /* reserve memory for <n> amount of data */
        data = malloc((len + 1) * sizeof(char));
        fread(data, sizeof(char), len, f);
        struct entry e = {id, len, data};
        list[inputLength] = e;
        inputLength++;        
    }    
    fclose(f);    
    f = fopen("boot.dat", "wb");    
    /* increase header length by 1 and write it back to file */
    struct header outhdr = {inhdr.id, inhdr.len + 1};
    fwrite(&outhdr, sizeof(struct header), 1, f);    
    /* write all pre existing data back to file */
    int outputLength = 0;
    while(outputLength < inputLength) {
        fwrite(&list[outputLength].id, sizeof(int), 1, f);
        fwrite(&list[outputLength].len, sizeof(int), 1, f);
        fwrite(&list[outputLength].data[0], sizeof(char), list[outputLength].len, f);
        outputLength++;
    }
    /* append the new one at the end */
    struct entry e;   
    e.id = newId;
    e.data = newdata;
    e.len = dataLen;
    fwrite(&e.id, sizeof(int), 1, f);
    fwrite(&e.len, sizeof(int), 1, f);
    fwrite(e.data, sizeof(char), e.len, f);        
    fclose(f);    
}

/* delete a file */
void deleteFile(int entryId) {
    FILE *f = fopen("boot.dat", "rb");       
    /* get the header */
    struct header inhdr;
    fread(&inhdr.id, sizeof(int), 1, f);
    fread(&inhdr.len, sizeof(int), 1, f);
    /* create a new list to store all existing entries */
    struct entry list[inhdr.len];    
    /* get the entries */
    int id, len = 0; char *data; 
    int inputLength = 0;
    while(inputLength < inhdr.len) {    
        fread(&id, sizeof(int), 1, f);
        fread(&len, sizeof(int), 1, f);
        /* reserve memory for <n> amount of data */
        data = malloc((len + 1) * sizeof(char));
        fread(data, sizeof(char), len, f);
        struct entry e = {id, len, data};
        list[inputLength] = e;
        inputLength++;        
    }    
    fclose(f);
    f = fopen("boot.dat", "wb");
    /* decrease header length nby 1 and write it back to file */    
    struct header outhdr;
    outhdr.len = inhdr.len - 1;
    fwrite(&outhdr.id, sizeof(int), 1, f);
    fwrite(&outhdr.len, sizeof(int), 1, f);    
    /* write all pre existed files back*/
    int outputLength = 0;
    while(outputLength < inputLength) {
        if(list[outputLength].id == entryId) {
            /* except the one to delete */
        } else {
            fwrite(&list[outputLength].id, sizeof(int), 1, f);
            fwrite(&list[outputLength].len, sizeof(int), 1, f);
            fwrite(&list[outputLength].data[0], sizeof(char), list[outputLength].len, f);
        }
        outputLength++;
    } 
    fclose(f);      
}

/* this is just for devel releases */
void createExample() {
    FILE *f = fopen("boot.dat", "wb");   
    // create a header with id=1 and len=2 
    struct header hdr = {1, 10};
    fwrite(&hdr, sizeof(struct header), 1, f);
    char *data;
    
    data = "ZShell v0.1 Alpha Demo";
    int id = 1;
    int len = strlen(data);    
    struct entry e = {id, len, data};
    fwrite(&e.id, sizeof(int), 1, f);
    fwrite(&e.len, sizeof(int), 1, f);
    fwrite(e.data, sizeof(char), strlen(e.data), f);
    
    id = 2; 
    data = "zs>";
    len = strlen(data); 
    e.id = id;
    e.data = data;
    e.len = len;
    fwrite(&e.id, sizeof(int), 1, f);
    fwrite(&e.len, sizeof(int), 1, f);
    fwrite(e.data, sizeof(char), e.len, f);
    
    id = 3; 
    data = "Powered by ZVM - Visit https://github.com/zarat/vm for more information"; 
    len = strlen(data); 
    e.id = id;
    e.data = data;
    e.len = len;
    fwrite(&e.id, sizeof(int), 1, f);
    fwrite(&e.len, sizeof(int), 1, f);
    fwrite(e.data, sizeof(char), e.len, f);
    
    id = 4; 
    data = "Available commands: info, help, exec, exit"; 
    len = strlen(data); 
    e.id = id;
    e.data = data;
    e.len = len;
    fwrite(&e.id, sizeof(int), 1, f);
    fwrite(&e.len, sizeof(int), 1, f);
    fwrite(e.data, sizeof(char), e.len, f);
    
    id = 5; 
    data = "info"; 
    len = strlen(data) + 1; 
    e.id = id;
    e.data = data;
    e.len = len;
    fwrite(&e.id, sizeof(int), 1, f);
    fwrite(&e.len, sizeof(int), 1, f);
    fwrite(e.data, sizeof(char), e.len, f);
    
    id = 6; 
    data = "help"; 
    len = strlen(data) + 1; 
    e.id = id;
    e.data = data;
    e.len = len;
    fwrite(&e.id, sizeof(int), 1, f);
    fwrite(&e.len, sizeof(int), 1, f);
    fwrite(e.data, sizeof(char), e.len, f);
    
    id = 7; 
    data = "exec"; 
    len = strlen(data) + 1; 
    e.id = id;
    e.data = data;
    e.len = len;
    fwrite(&e.id, sizeof(int), 1, f);
    fwrite(&e.len, sizeof(int), 1, f);
    fwrite(e.data, sizeof(char), e.len, f);
    
    id = 8; 
    data = "exit"; 
    len = strlen(data) + 1; 
    e.id = id;
    e.data = data;
    e.len = len;
    fwrite(&e.id, sizeof(int), 1, f);
    fwrite(&e.len, sizeof(int), 1, f);
    fwrite(e.data, sizeof(char), e.len, f);
    
    id = 9; 
    data = "unknown command: "; 
    len = strlen(data); 
    e.id = id;
    e.data = data;
    e.len = len;
    fwrite(&e.id, sizeof(int), 1, f);
    fwrite(&e.len, sizeof(int), 1, f);
    fwrite(e.data, sizeof(char), e.len, f);
    
    id = 10; 
    data = "command: "; 
    len = strlen(data); 
    e.id = id;
    e.data = data;
    e.len = len;
    fwrite(&e.id, sizeof(int), 1, f);
    fwrite(&e.len, sizeof(int), 1, f);
    fwrite(e.data, sizeof(char), e.len, f);
    
    fclose(f);
}

int main(int argc, char **argv) {
    
    /* by default just create an empty storage struct */
    if(argc == 2) {
        createExample();    
    }    
    
    /* 
    this 2 functions should only be available in developer versions
    compile with -D _DEVEL to implement these
    */
    if(argc == 1) {
        create();
    }  
      
    if(argc == 3) {
        char *data;
        create();
        getch();
        data = "What the fuck is going on here?";        
        createFile(15, data, strlen(data));    
        getch();    
        deleteFile(15);
        getch(); 
        data = "ZShell v0.1 Alpha Demo";  
        createFile(1, data, strlen(data));
    } 
}