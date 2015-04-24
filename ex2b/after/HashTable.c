/*
 * HashTable.c
 * Author: Olesya Shapira 319346565
 */

#include "HashTable.h"

Table *createTable(int size, int dType, int listLength)
{
    int i;
    Table *myTable = (Table *)malloc(sizeof(Table));

    if (myTable == NULL)
    {
        printf("Malloc error!");
        return NULL;
    }

    myTable->arr = malloc(sizeof(Object *)*size);

    if (myTable->arr == NULL)
    {
        printf("Malloc error!");
        return NULL;
    }

    for (i = 0; i < size; i++) //Put Null in every cell in array
        myTable->arr[i] = NULL;

    if (size > 1) //The size of the table have to be at least 1
        myTable->originalSize = size;
    else return NULL;

    myTable->currentSize = size;

    if (listLength > 0) //The link list length have to be at least 1
        myTable->listSize = listLength;
    else return NULL;

    if (dType == 0 || dType == 1) //Data type of the table have to be int or string
        myTable->dataType = dType;
    else return NULL;

    myTable->d = myTable->currentSize / myTable->currentSize;

    return myTable;
}

void freeTable(Table *table)
{
    int i;

    if (table == NULL)
        return;

    for (i = 0; i < table->currentSize; i++) //Free all the objects in array
        freeList(table->arr[i], table->dataType); //Free the linked list of objects

    free(table->arr); // Free the array
    free(table);
}

void freeList(Object *obj, int dType)
{
    if (obj == NULL)
        return;

    if (obj->next)
    {
        freeList(obj->next, dType);
    }

    freeObject(obj, dType);
}

Object *createObject(void *data, int dType, int weight, char *ip, int port)
{
    Object *myObject;
    myObject = (Object *)malloc(sizeof(Object));
    char *cData;
    char *newIp;

    if (myObject == NULL)
    {
        printf("Malloc error!");
        return NULL;
    }

    if (data == NULL)
    {
        free(myObject);
        return NULL;
    }

    if (dType == STR_TYPE) //String type
    {
        cData = malloc((strlen((char *)data) * sizeof(char)) + 1);
        strcpy(cData, (char *)data);
        myObject->vertex = cData;
    }

    if (ip != NULL)
    {
        newIp = malloc((strlen((char *)ip) * sizeof(char)) + 1);
        strcpy(newIp, ip);
        myObject->ip = newIp;
    }

    if (myObject->vertex == NULL)
    {
        printf("Malloc error!");
        return NULL;
    }

    myObject->next = NULL;
    myObject->indx = ERR;
    myObject->weight = weight;
    myObject->via = NULL;
    myObject->port = port;
    myObject->neighbor = myObject->vertex;
    return myObject;
}

void freeObject(Object *obj, int type) //Free the data due to it's type
{
    if (type == STR_TYPE)
        free((char *)obj->vertex);
    else
        free((int *)obj->vertex);

    free((char *)obj->ip);

    free(obj);
}


int add(Table *table, void *data, void *bdata, int weight, char *s, char *ip, int port)
{
    int indx, i, count;
    Object *aObject, *bObject;

    if (table == NULL)
        return ERR;

    if (isEqual(STR_TYPE, data, s))
        aObject = createObject(data, table->dataType, INFINITY, ip, port);
    else aObject = createObject(data, table->dataType, 0, ip, port);

    if (bdata != NULL)
    {
        bObject = createObject(bdata, table->dataType, weight, ip, port);

        if (aObject == NULL || bObject == NULL)
            return ERR;
    }
    else bObject = NULL;

    indx = table->d * strHashFun((char *)data, table->originalSize);

    if (!table->arr[indx]) //Default place is Empty (first time only)
    {
        table->arr[indx] = aObject;
        table->arr[indx]->next = bObject;
        aObject->indx = indx;
    }
    else
    {
        if (isEqual(table->dataType, data, table->arr[indx]->vertex) == 0) //Only if same vertex
            count = addToList(table, aObject, bObject, indx);
        else
        {
            count = table->listSize;
        }

        if (count == table->listSize) //Need to go to another index
        {
            if (table->d == 1) //First time the table has to grow
            {
                table = resizeTable(table);
                indx = indx * table->d;
            }

            if (table->d > 1)
            {
                for (i = indx; i < indx + table->d; i++) //Looking for a free cell in array
                {
                    if (!table->arr[i]) //First cell is free
                    {
                        table->arr[i] = aObject;
                        aObject->indx = i;
                        table->arr[i]->next = bObject;
                        return i;
                    }
                    else
                    {
                        if (isEqual(table->dataType, data, table->arr[indx]->vertex) == 0) //Only if same vertex
                        {
                            count = addToList(table, aObject, bObject, i); //Need to add to linked list

                            if (count < table->listSize)
                            {
                                aObject->indx = i;
                                return i;
                            }
                        }
                    }
                }

                if (i == indx + table->d) //No place in the table, need to grow
                {
                    resizeTable(table);
                    freeObject(aObject, STR_TYPE);
                    freeObject(bObject, STR_TYPE);

                    return add(table, data, bdata, weight, s, ip, port);
                }
            }
        }
    }

    return indx;
}

int addToList(Table *table,  Object *aObject, Object *bObject, int indx)
{
    Object *temp;
    int count = 1;
    temp = table->arr[indx];

    while (temp->next) //Counting how many object already in linked list
    {
        count++;
        temp = temp->next;
    }

    if (count < table->listSize) //There are still free place in linked list
    {
        temp->next = bObject;
        freeObject(aObject, STR_TYPE);
    }

    return count;
}

Table *resizeTable(Table *table)
{
    Object *temp;
    Object *temp1;

    int i, j;
    table->arr = realloc(table->arr, (sizeof(Object *)*table->currentSize) * 2);

    if (table->arr == NULL)
    {
        printf("Realloc error!");
        return NULL;
    }

    for (i = table->currentSize; i < 2 * table->currentSize; i++) //Putting Null in all new cells
        table->arr[i] = NULL;

    for (i = (table->currentSize - 1); i > 0; i--)
    {
        temp = table->arr[i];

        if (temp)
        {
            table->arr[i * 2] = temp;
            temp->indx = i * 2;
            table->arr[i] = NULL;
            temp1 = temp->next;

            for (j = 0; j < table->listSize; j++) //Running on all linked lists to change the index of all objects
            {
                if (temp1)
                {
                    temp1->indx = i * 2;
                    temp1 = temp1->next;
                }
                else break;
            }
        }
    }

    table->currentSize = 2 * table->currentSize;
    table->d = table->currentSize / table->originalSize;

    return table;
}

void printTable(Table *table)
{

    int i;

    if (table == NULL)
    {
        printf("Error! The table is NULL\n");
        return;
    }

    printf("Destination\tVia\t\tCost\n");

    for (i = 0; i < table->currentSize; i++)
    {
        if (table->arr[i])
        {
            if (table->arr[i]->weight == -INFINITY)
                printf("%s\t\t %s\t\t-infinity\n", (char *)table->arr[i]->vertex, (char *)table->arr[i]->neighbor);
            else
                printf("%s\t\t %s\t\t%d\n", (char *)table->arr[i]->vertex, (char *)table->arr[i]->neighbor, table->arr[i]->weight);
        }
    }
}


Object *search(Table *table, void *data)
{
    int i, indx;
    Object *temp;
    temp = NULL;

    indx = table->d * strHashFun((char *)data, table->originalSize);

    for (i = indx; i < indx + table->d; i++)
    {
        if (table->arr[i])
        {
            if (isEqual(table->dataType, table->arr[i]->vertex, data) == 0) //First looking on the first cell
            {
                return table->arr[i];
            }
        }
    }

    return temp;
}

int isEqual(int type, void *data1, void *data2)
{
    int value = ERR; //Value=0 if equal, -1 if not

    if (data1 == NULL || data2 == NULL)
        return value;

    if (type == STR_TYPE) //String type
    {
        value = strcmp((char *)data1, (char *)data2);
    }

    return value;
}

int strHashFun(char *key, int origSize) //Hash function
{
    int sum = 0, i, value = ERR;

    if (key == NULL)
        return value;

    for (i = 0; i < strlen(key); i++)
        sum += (int)key[i];

    if (sum >= 0)
        value = sum % origSize;

    return value;
}


