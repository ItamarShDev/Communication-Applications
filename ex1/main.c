#include "GenericHashTable.h"
#include <stdio.h>
#include <stdlib.h>
int main(int argc, char **argv)
{
    int a = 2, b = 6, c = 11;
    Object* obj;
    char* ch;
    void *p;
    void *t;
    void *t3 = &c;
    //p=;
    ch = "hey";
	t=&a;
	p=&b;
    Table* table; 
	table= createTable(4,0, 2);
    add(table, t);
    printTable(table);
    add(table, t);
    add(table, t);
    printTable(table);
    // printf("%d",isEqual(1,t,t);
    // removeObj(table,t);
    obj=search(table,t);
   // printTable(table); 

    // t = &a;
    // t2 = &b;
    // t3 = &c;	
    add(table, t);
    printTable(table);
    add(table, t);
    printTable(table);
    add(table, t);
    printTable(table);
    add(table, t);
    printTable(table);
    add(table, t);
    printTable(table);
    add(table, t);
    printTable(table);
    add(table, t);
    printTable(table);
    add(table, p);
    printTable(table);
	obj= search(table,p);
    // freeTable(table);
    // printTable(table);

    /*
    //printf("%d\n",removeObj(table,t2) );
      printf("\n");
    t = &a;
    t2 = &b;
    t3 = &c;
    add1 = add(table, t);
    add2 = add(table, t2);
    add3 = add(table, t3);
    printTable(table);*/
    freeTable(table);
    return 1;
}
