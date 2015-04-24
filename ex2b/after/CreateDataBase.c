
/*
 * CreateDataBase.c
 * Author: Olesya Shapira 319346565
 */

//This program read from file and place it into Data Base
#include "CreateDataBase.h"


Table *readFile(const char *filename, char *s)
{
    FILE *file = fopen ( filename, "r" );
    char line [128];
    int i;
    Table *table;
    table = NULL;

    if (file != NULL)
    {
        fgets(line, sizeof line, file);
        num = atoi(line);
        table = createTable(num, STR_TYPE, num);
        table->vertixNum = num;

        for (i = 0; i < num; i++)
        {
            fgets(line, sizeof line, file);
            insertRouters(table, line, s);

        }

        while (fgets(line, sizeof line, file ) != NULL) /* read a line */
        {
            insertToDataBase(table, line, s);
        }

        fclose (file);
    }
    else
    {
        perror (filename); // Error with file open/close
    }

    return table;
}

//This function add all the routers names with ip and port
void insertRouters(Table *table, char *line, char *s)
{
    char *ip, *name, *token;
    char *arr[N];
    int i, port;

    token = strtok(line, " ");
    name = token;

    for (i = 0; i < N - 1; i++)
    {
        token = strtok(NULL, " ");
        arr[i] = token;
    }

    ip = arr[0];

    if (arr[1] != NULL) //Check if is not empty line
        port = atoi(arr[1]);
    else return;

    add(table, name, NULL, 0, s, ip, port);

}

//This function add neighbors to all the routers
void insertToDataBase(Table *table, char *line, char *s)
{
    char *adata, *bdata, *token;
    Object *temp;
    char *arr[N];
    int weight, i = 0;

    token = strtok(line, " ");
    adata = token;

    for (i = 0; i < N - 1; i++)
    {
        token = strtok(NULL, " ");
        arr[i] = token;
    }

    bdata = arr[0];

    if (arr[1] != NULL) //Check if is not empty line
        weight = atoi(arr[1]);
    else return;

    temp = search(table, bdata);

    if (temp != NULL)
        add(table, adata, bdata, weight, s, temp->ip, temp->port);

    temp = search(table, adata);

    if (temp != NULL)
        add(table, bdata, adata, weight, s, temp->ip, temp->port);

}

