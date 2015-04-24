#include <stdio.h>
#include <stdlib.h>
#include "GenericHashTable.h"
#define NEG_CRCLE 0;
#define NO_CRCLE 1;
#define EXIST 1;
#define NOTEXIST 0;
void createDB(Table *t,FILE* fp);	
void FreeTokens(char** tokens);
char** ReadTokens(char* stream);
int  findShortestPath(Table *t, const char* src);
char* findNeighbor(Table* t, void* data,const char* source);
void toString(Table * t,const char* source);
void set_to_infinity(Table* t);