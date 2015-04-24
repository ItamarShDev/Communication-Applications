#include "GenericHashTable.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#define NULL_PTR -4
#define NO_ARRAY -2
#define LIST_FULL -3
#define ERROR -1
/**
* The function gets the original size and the type of the data in the table elements.
* it initializes the Table struct members.
* On success, the function returns a pointer to the new created Table, otherwise, it return NULL.
*/
Table* createTable(int size, int dType, int listLength)
{
	int i;
	//memory allocation
	Table *t = malloc(sizeof(Table));
	t->arr = malloc(size*sizeof(Object*));

	for(i=0; i<size; i++)
	{
		t->arr[i]=NULL;   //initialize to NULL's
	}

	//set the variables
	t->listLength = listLength;
	t->OriginSize = size;
	t->type = dType;
	t->d = size/t->OriginSize;
	t->curr_size = size;
	return t;
}
/**
* This function creates an object and return the pointer to it or NULL if there is some error.
*/
Object* createObject(int type,const void *data)
{
	char* tempc;//case its a word
	Object *temp;

	if (!data)//if there is no data
		return NULL;

	//allocate memory
	temp = malloc(sizeof(Object));

	switch(type)
	{
		case INT_TYPE:
			temp->data = (int*)data;
			break;

		case STR_TYPE:
			tempc = malloc(sizeof(char*)*(strlen((char*)data)+1));
			strcpy(tempc, (char*)data);
			temp->data = tempc;
			break;
	}

	temp->next=NULL;
	temp->prev= NULL;
	temp->pi = NULL;
	return temp;
}
/**
* The function adds data to the hashtable (as described in the exe definition)
* On success, the function returns the array index of the added data,s otherwise, it return -1.
*/
int add(Table *table,const void *data,const void* b_data, int weight)
{
	int i,mod, ret, tmp;
	Object *obj,*b_obj;

	if(data&&b_data)
	{
		obj = createObject(table->type, data);
		b_obj = createObject(table->type, b_data);
		b_obj->weight=weight;
	}

	//calculate the Hash value
	switch (table->type)
	{
		case INT_TYPE://case int
			mod = (table->d) * intHashFun((int *)data, table->OriginSize);
			break;

		case STR_TYPE://case string
			mod = (table->d) * strHashFun((char *)data, table->OriginSize);
			break;


		default:
			perror("Wrong Input");
			break;
	}

	tmp = insertObject(table,obj, b_obj, mod);//find a place to put it in

	switch (tmp)
	{
		case NO_ARRAY:
			printf("ERROR: arr's memory was never allocated\n");
			ret = NO_ARRAY;
			break;

		case NULL_PTR:
			printf("ERROR: Object or Table is NULL\n");
			ret = NULL_PTR;
			break;

		case LIST_FULL:
			for(i=mod; i<mod+table->d; i++) //run on the permitted cells
			{
				if(i==table->curr_size)
				{
					freeObject(b_obj,table->type);
					freeObject(obj,table->type);
					break;
				}

				ret= insertObject(table,obj,b_obj,i);

				if (ret>=0)
					return i;
			}

			if(i==mod+table->d)//case non has place resize the array
			{
				resizeHash(table);
				freeObject(obj,table->type);
				freeObject(b_obj,table->type);
				return add(table,data,b_data,weight);
			}

			freeObject(obj,table->type);
			freeObject(b_obj,table->type);
			break;

		default:
			ret = tmp;
			break;
	}

	return ret;

}
int addObject(Table *table,const void *data,int port,char* ip)
{
	int i,mod, ret, tmp;
	Object *obj;

	if(data)
	{
		obj = createObject(table->type, data);
		obj->port = port;
		obj->ip = ip;
	}

	//calculate the Hash value
	switch (table->type)
	{
		case INT_TYPE://case int
			mod = (table->d) * intHashFun((int *)data, table->OriginSize);
			break;

		case STR_TYPE://case string
			mod = (table->d) * strHashFun((char *)data, table->OriginSize);
			break;


		default:
			perror("Wrong Input");
			break;
	}

	tmp = insertObject(table,obj, NULL, mod);//find a place to put it in

	switch (tmp)
	{
		case NO_ARRAY:
			printf("ERROR: arr's memory was never allocated\n");
			ret = NO_ARRAY;
			break;

		case NULL_PTR:
			printf("ERROR: Object or Table is NULL\n");
			ret = NULL_PTR;
			break;

		case LIST_FULL:
			for(i=mod; i<mod+table->d; i++) //run on the permitted cells
			{
				if(i==table->curr_size)
				{
					freeObject(obj,table->type);
					break;
				}

				ret= insertObject(table,obj,NULL,i);

				if (ret>=0)
					return i;
			}

			if(i==mod+table->d)//case non has place resize the array
			{
				resizeHash(table);
				freeObject(obj,table->type);
				return addObject(table,data,port, ip);
			}

			freeObject(obj,table->type);
			break;

		default:
			ret = tmp;
			break;
	}

	return ret;

}
/*
*this function insert the object to its place according to the factors
*/

int insertObject(Table *t, Object *obj,Object *b_obj, int loc)
{
	Object *temp;

	if (!t || !obj)//case the are doesnt exist
		return NULL_PTR;

	if (!t->arr)//if no hash to be found
		return NO_ARRAY;

	if (t->arr[loc]==NULL)//if the place is empty, let it be this object
	{
		t->arr[loc]= obj;

		if(b_obj!=NULL)
			t->arr[loc]->next=b_obj;

		obj->index=loc;
		free(obj->prev);
		return loc;
	}

	else if (!isEqual(t->type,t->arr[loc]->data, obj->data))//case this is the same Object
	{
		int flag=1;
		temp = t->arr[loc];

		while (temp->next)
		{
			if(!isEqual(t->type,temp->data,b_obj->data))
				flag = 0;

			temp = temp->next;
		}

		if(flag)
		{
			addToList(t->arr[loc], b_obj);
			freeObject(obj,t->type);
		}

		return loc;
	}

	else if(isEqual(t->type,t->arr[loc]->data, obj->data))//case not the same
		return LIST_FULL;

	return ERROR;
}
/*
* this function adds an object to the end of the list
*
*/
void addToList(Object* list, Object* obj)
{
	Object *temp = list;

	while (temp->next)
	{
		temp = temp->next;   //go to the end
	}

	if(temp)
	{
		obj->index=temp->index;
		temp->next = obj;
		obj->prev= temp;
	}
}
//this function doubles the array size
//then call reinitHash to re-order it
void resizeHash(Table* t)
{
	int i;
	t->arr = realloc(t->arr,2*t->curr_size*sizeof(Object*));//re-allocation, ttwice the size

	for(i=t->curr_size; i<2*t->curr_size; i++) //initialize the new cells to NULL
		t->arr[i] = NULL;

	if (t->arr)
	{
		reinitHash(t);//re-order the cells
	}

	else printf("Realloc Error\n");
}
/*reinsert the objects
*put any existing list in cell i at cell 2i
*this function runs backwards to prevent double cell moving
*/
void reinitHash(Table* t)
{
	int i;
	Object *obj;

	for (i = t->curr_size-1; i >0; i--)
	{
		if (t->arr[i])
		{
			obj = t->arr[i];
			obj->index*=2;
			t->arr[2 * i] = obj;
			t->arr[i] = NULL;
		}
	}

	t->curr_size *= 2;
	t->d *=2;
}
/**
* returns the hash value of an integer, which is key mod origSize
*/
int intHashFun(int *key, int origSize)
{
	int ret;

	if (key)
	{
		ret = (*key) % origSize;
	}

	else ret = NULL_PTR;

	return ret;
}
/**
* returns the hash value of an string, which is m mod origSize, where m is the sum of the ascii value of all the
* character in key.
*/
int strHashFun(char *key, int origSize)
{
	int count=0, i = 0;

	while (key[i] != '\0')
	{
		count += key[i];
		i++;
	}

	return count % origSize;
}
/**
* check the equality of the data of two objects. The implementation is different depending the type of the data.
* the function returns 0 if they are equal or some other value if they are not equal.
*/
int isEqual(int type,const void *data1,const void *data2)
{
	int ret = -1,i;

	switch (type)
	{
		case INT_TYPE:
			if (*(int *)data1 == *(int *)data2)
				ret = 0;

			break;

		case STR_TYPE:
			i=strcmp((char *)data1 ,(char *)data2);

			if (!i)
				ret = 0;

			break;
	}

	return ret;
}
/**
* The function release all the allocated members of struct Table.
*/
void freeTable(Table *table)
{
	int i;

	for (i = 0; i < table->curr_size; i++)
	{
		freeObject(table->arr[i],table->type);//free's the list
	}

	free(table->arr);
	free(table);
}
/**
* This function frees an object,  note the in the case of STR_TYPE, the data
* of the object should also be freed.
*incase of a list this function free the list started with the inputed object
*/
void freeObject(Object* obj, int type)
{
	if(!obj)
		return;

	if(obj->next)
		freeObject(obj->next,type);

	if(!obj->prev)
		free(obj->prev);

	switch(type)
	{
		case STR_TYPE:
			free((char*)obj->data);
			break;
	}

	free(obj);
}
/**
* The function print the table (the format is in the exe definition)
*/
void printTable(Table *table)
{
	int i = 0;

	if(table)
	{
		for (i = 0; i < table->OriginSize*table->d; i++)
		{
			printf("------ \n");

			if(table->arr[i])
			{
				// if(table->arr[i]->pi)
				printf("[%s] (pi:%s,d:%d,port:%d,ip:%s)   \t", (char*)table->arr[i]->data,(char*)table->arr[i]->pi,table->arr[i]->weight,table->arr[i]->port, table->arr[i]->ip);
				// else printf("[%s] (d:%d)         \t", (char*)table->arr[i]->data,table->arr[i]->weight);
				printList(table,i);
				printf("\n------");

			}

			printf("\n");
		}
	}

	printf("\n");
}
/*
*this funtion prints the list in a gven array cell
*/
void printList(Table *table,int i)
{
	int j;
	Object* obj;

	if (table->arr[i]->next)
	{
		obj = table->arr[i]->next;

		for (j = 0; j <=countList(obj); j++)
		{
			switch(table->type)
			{

				case INT_TYPE:
					printf("%d\t", *(int*)obj->data );
					break;

				case STR_TYPE:
					printf("%s\t", (char *)obj->data );
					break;
			}

			printf("%d\t%d", obj->weight,obj->_rec);

			if (obj->next)
			{
				printf("-->\t");
			}

			obj = obj->next;
		}
	}
}
//this function count the number of objects in a given list
int countList(Object* obj)
{
	int ret=0;

	if(obj)
	{
		while(obj)
		{
			ret++;
			obj=obj->next;
		}
	}

	return ret;
}
/**
* The function search    for an object that its data is equal to data and returns a pointer to that object.
* If there is no such object or in a case of an error, NULL is returned.
*USING DOUBLE SIDED LIST
*/
Object* search(Table* table, const void* data)
{
	int i, mod;
	Object *obj;

	switch(table->type)
	{
		case INT_TYPE:
			mod=table->d*intHashFun((int*)data, table->OriginSize);
			break;

		case STR_TYPE:
			mod = (table->d)*strHashFun((char *)data, table->OriginSize);
			break;
	}

	for(i=mod; i<mod+table->d; i++)
	{
		if(table->arr[i])
		{
			obj=table->arr[i];

			if(!isEqual(table->type,data,obj->data))
			{
				return obj;
			}
		}
	}

	return NULL;
}

/**
* The function removes the Object which its data equals to data, if there are more than one, it removes the first one.
* On success, the function returns the array index of the removed data, otherwise, it returns -1.
* -1 is also returned in the case where there is no such object.
*/
int removeObj(Table* table,const void* data)
{
	int index = ERROR;
	Object *t_prev, *temp,*t_next;
	temp = search(table,data);

	if(!temp)
		return ERROR;

	index =temp->index;

	if(!temp->prev)
	{
		t_next = temp->next;
		table->arr[index]=t_next;

		if(temp->next)
			t_next->prev = NULL;
	}

	else
	{
		t_prev=temp->prev;
		t_prev->next=temp->next;
		freeObject(temp,table->type);
	}

	return index;
}
int getWeight(Table* t,const void* a,const void* b)
{
	Object* obj;
	obj = search(t,a);

	while(obj)
	{
		if(!isEqual(t->type,obj->data,b))
			return obj->weight;

		obj=obj->next;
	}

	return -1;
}