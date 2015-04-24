#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "BellmanFord.h"
#ifdef GenericHashTable
#include "GenericHashTable.h"
#endif

/*creates the database in a hash function we have built in the first ecxersize*/
void createDB(Table* t,FILE* fp)
{
	int i,w,k;//to use in the loops
	char line[128], *temp;
	char *a,*b;
	char* tokens[3];

	while(fgets(line,sizeof(line),fp))//read file line by line
	{
		temp = strtok(line, " ");//get all word on by one
		k=0;

		while (temp!=NULL)//as long there are words
		{
			tokens[k] = temp;
			temp = strtok(NULL, " ");
			k++;
		}

		/*for days ive been searching for a valid way to ignore the empty lines
		and none have worked! so, filled with anger and dispare ive stayed
		with this ugly condition that just works
		sorry*/
		if(strlen(tokens[1])>0&&strlen(tokens[0])>0&&strlen(tokens[2])>2)
		{
			a = tokens[0];
			b = tokens[1];

			if(strcmp(a,b)==0)// id the same data in both, ignore
				continue;

			w = atoi(tokens[2]);
			//add both edges
			i = add(t,a,b,w);
			i = add(t,b,a,w);
		}
	}

	fclose(fp);

	for(i=0; i<t->curr_size; i++) //from i: 1 to v-s
	{
		if(t->arr[i])//if it exist
			t->arr[i]->weight=INT_MAX; //d[v]=infinity
	}
}

/*free the tokens*/
void FreeTokens(char** tokens)
{
	int i;

	for(i = 0; tokens[i]!=NULL; i++)
		free(tokens[i]);

	free(tokens);
}

//Bellman-Ford Algorithm
int  findShortestPath(Table *t, const char *src)
{
	int i,k,j, ret = NO_CRCLE;
	Object *u,*v,*temp;

	for(j=0; j<t->OriginSize; j++) //run on the array N times
	{
		for(i=0; i<t->curr_size; i++)
		{
			if(t->arr[i])
			{
				u=t->arr[i];
				temp = u;

				for(k=0; k<countList(t->arr[i]); k++)
				{
					v=search(t,temp->data);

					//INT_MAX+1 = -1 thus it's needed to correct this by checking each
					if(temp->weight==INT_MAX)
					{
						if(v->weight==INT_MAX)
							v->weight=INT_MAX;
					}
					//if d[v]>v[u]+w(u,v)
					else if(v->weight>(temp->weight+temp->weight))
					{
						v->weight=(temp->weight+temp->weight);
						v->pi = (char*)temp->data;
					}

					temp=temp->next;
				}
			}
		}
	}

	//find neg circles
	for(i=0; i<t->curr_size; i++)
	{
		if(t->arr[i])
		{
			u=t->arr[i];
			temp = u;

			//tun on the list
			for(k=1; k<countList(t->arr[i]); k++)
			{
				v=search(t,temp->data);

				if(u->weight==INT_MAX)
				{
					if(v->weight==INT_MAX)
						v->weight=INT_MAX;
				}
				else if(v->weight>(u->weight+temp->weight))
				{
					ret= NEG_CRCLE;
				}

				temp=temp->next;
			}
		}
	}

	return ret;
}
//print the direction table
void toString(Table * t,const char* source)
{
	int i;
	char* c;

	if(!source||!t)
		return;

	printf("To\tVia\tCost\n");

	for(i=0; i<t->curr_size; i++)
	{
		if(t->arr[i])
		{
			c = findNeighbor(t,t->arr[i]->data,source);

			if(t->arr[i]->weight==-1)
				printf("%s\t%s\t-infinity\n",(char*)t->arr[i]->data,c);
			else if(t->arr[i]->weight==INT_MAX)
				printf("%s\t%s\tinfinity\n",(char*)t->arr[i]->data,c);
			else printf("%s\t%s\t%d\n",(char*)t->arr[i]->data,c,t->arr[i]->weight);
		}
	}
}
/*using a recursive run to find the neighbore for the selected vertex*/
char* findNeighbor(Table* t, void* data,const char* source)
{
	Object* src;

	if(!data||!source||!t)
		return NULL;

	src = search(t, data);

	if(!(src->pi))
		return (char*)data;
	else if(!strcmp(src->pi,source)||data==src->pi)
		return (char*)data;

	return  findNeighbor(t,src->pi,source);
}
//set all list to -infinity
void set_to_infinity(Table* t)
{
	int i;

	for(i=0; i<t->curr_size; i++)
	{
		if(t->arr[i])
			t->arr[i]->weight = -1;
	}
}