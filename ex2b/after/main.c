/*
 * main.c
 *
 *  Author: Olesya Shapira 319346565
 */


#include "router.h"

int main( int argc, char *argv[] )
{
    Table *table;

    if (argc < 4) //Get file name and source as arguments from user
    {
        printf("Not enough arguments! Please add file name, name of the router and number of trials\n");
        return 1;
    }

    table = readFile(argv[1], argv[2]); //Read file and place the information in Hash table

    if (table == NULL)
        exit(1);

    prepare(argv[2], atoi(argv[3]), table); // Go to main function to start all the fun! =)
    freeTable(table);
    return 0;
}
