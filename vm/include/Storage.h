/*

Author: Manuel Zarat
Date: 12.12.2019
Copyleft

A class that represents the virtual machines memory
Right now the storage only read/write to a binary file
when an address in memory is stored using special functions, data on disk gets updated too

*/

#include <stdio.h>
#include <stdlib.h>

struct header {
    int id;
    int len; // total length == no of files!
};

struct entry {
    int id; // id of the block      
    int len; // length of data        
    char *data; // pointer to current data  
};


void readStorage(char *src) {    

    FILE *f = fopen(src, "rb");           
    /* get the header */
    struct header inhdr;
    fread(&inhdr.id, sizeof(int), 1, f);
    fread(&inhdr.len, sizeof(int), 1, f);    
    /* create a new list to store all existing entries */
    struct entry list[inhdr.len];       
    /* get the entries */
    int id, len = 0; 
    char *data; 
    int inputLength = 0;
    while(inputLength < inhdr.len) {    
        fread(&id, sizeof(int), 1, f);
        fread(&len, sizeof(int), 1, f);
        /* reserve memory for <n> amount of data */
        data = malloc((len + 1) * sizeof(char));
        fread(data, sizeof(char), len, f);
        struct entry e = {id, len, data};
        list[inputLength++] = e;        
    }    
    /* append them to the vm's memory */    
    int outputLength = 0;
    while(outputLength < inputLength) {
        insertFirst(list[outputLength].id, &list[outputLength].data[0], list[outputLength].len);
        outputLength++;
    }        
    fclose(f);
}

void createFile(char *src, int newId, char *newdata, int dataLen) {
    FILE *f = fopen(src, "rb");       
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
    f = fopen(src, "wb");    
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

void deleteFile(char *src, int entryId) {
    FILE *f = fopen(src, "rb");       
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
    f = fopen(src, "wb");
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