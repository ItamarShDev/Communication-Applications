/*
 * HashTable.h
 *
 *  Created on: Nov 4, 2014
 *      Author: Olesya Shapira 319346565
 */

#ifndef HASHTABLE_H_
#define HASHTABLE_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define INT_TYPE 0
#define STR_TYPE 1
#define ERR -1
#define INFINITY 1000000

typedef struct Object
{
    void *vertex, *neighbor;
    struct Object *next;
    char *via;
    int indx;
    int weight;
    char *ip;
    int port;
    int socketfd;
    int sent;
} Object;

typedef struct Table
{
    Object **arr;
    int originalSize, listSize, currentSize;
    int dataType, d;
    int vertixNum;
} Table;

Table *createTable(int size, int dType, int listLength);
/**
* The function gets the original size and the type of the data in the table elements.
* it initializes the Table struct members.
* On success, the function returns a pointer to the new created Table, otherwise, it return NULL.
*/

void freeTable(Table *table);
/**
* The function release all the allocated members of struct Table.
*/

int add(Table *table, void *data, void *bdata, int weight, char *s, char *ip, int port);
/**
* The function adds data to the hash table
* On success, the function returns the array index of the added data, otherwise, it return -1.
*/

int addToList(Table *table,  Object *aObject, Object *bObject, int indx);
/**
 * The function adds object to linked list, up to maximum list size
 */

Table *resizeTable(Table *table);
/*
 * The function multiply existing table by 2. Every cell in array is moved to cell x 2
 */

Object *search(Table *table, void *data);
/**
* The function search for an object that its data is equal to data and returns a pointer to that object.
* If there is no such object or in a case of an error, NULL is returned.
*/

void printTable(Table *table);
/**
* The function print the table in format    vertex      via     weight
*/

Object *createObject(void *data, int dType, int weight, char *ip, int port);
/**
* This function creates an object and return the pointer to it or NULL if there is some error.
*/

void freeObject(Object *obj, int type);
/**
* This function frees an object
*/

void freeList(Object *obj,  int dType);
/*
 * This function is helping to free linked list of objects
 */

int isEqual(int type, void *data1, void *data2);
/**
* This function check's the equality of the data of two objects. The implementation is different depending the type of the data.
* The function returns 0 if they are equal or some other value if they are not equal.
*/

int strHashFun(char *key, int origSize);
/**
* Returns the hash value of an string, which is m mod origSize, where m is the sum of the ascii value of all the
* character in key.
*/

#endif /* HASHTABLE_H_ */
