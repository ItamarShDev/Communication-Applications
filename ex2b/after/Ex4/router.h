#ifndef ROUTER_H_
#define ROUTER_H_

#include <sys/socket.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>
#include "CreateDataBase.h"

#define LEN 256

//This function is the main function -creates the threads and start the work
void prepare(char *name, int tr, Table *t);
//This function is responsible for creating client, to connect and send the DV
void *sender(void *nei);
//This function is responsible for creating server, to get DV from neighbors
void *receiver(void *nei);
//Thread calculator check if all sent, then call to relax, if all sent 0 finishing the program
void *calculator(void *nei);
//This function recalculating the weight to each vertex in graph
void relax();
//This function checking for each neighbor if sent 0. return 1 if all sent 0
int check();
//This function checking for each neighbor if sent something. return 1 if all sent
int checkAllSent();
//This function updates the DV for neighbor who sent his dv
void update(int arr[], char *neighbor);
//This function get place in array and need to find the suitable vertex in table
char *find(int n);

#endif /* ROUTER_H_*/