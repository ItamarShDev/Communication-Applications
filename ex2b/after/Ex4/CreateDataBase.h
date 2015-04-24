/*
 * CreateDataBase.h
 *
 *  Created on: Nov 28, 2014
 *      Author: Olesya Shapira 319346565
 */

//This program read from file and place it into Data Base

#ifndef CREATEDATABASE_H_
#define CREATEDATABASE_H_

#include <stdio.h>
#include <stdlib.h>
#include "HashTable.h"
#define N 3

int num;
/* This function reads from the given file */
Table *readFile(const char *filename, char *s);

/*This function add all the routers names with ip and port */
void insertRouters(Table *table, char *line, char *s);

/*This function insert the data from file to Hash Table*/
void insertToDataBase(Table *table, char *line, char *s);

#endif /* CREATEDATABASE_H_ */
