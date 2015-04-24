//#include "BellmanFord.h"
// #include "GenericHashTable.h"
#include "BellmanFord.h"
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <limits.h>
//for debugging
void printBuf(int buf[]);

int comp(int arr[], int arr1[]);
//actual methods
int all_sent();
char* findMyIP(int num);//to find ones IP
char* findMyName(int num);//to find ones name
int calc_name_ascii(char const *name);//convert the name to ascii
int findMyPort(int num);//finds the port
char* find_by_place(int place);//returns its name according to the place on the array
void* calculate(void* count);//the BF
void create_threads(int n);//create the threads
void* thread_op(void* count);//decides if it to send or receive
void updateDV(int buf[], char* n_name);//
int check_all_sent_zero();
int all_data_received = 0;
int calc_finished = 0;
int num, loop;
int exist;
int close_program;
Table *table;
const char *src;
int num_of_edges;
int *sockets;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t calc = PTHREAD_COND_INITIALIZER;
pthread_cond_t sender = PTHREAD_COND_INITIALIZER;
int main(int argc, char const *argv[])
{
	int n, i;
	size_t size = 0;
	char     *line, *t_line, *token, *token2, *token3;
	FILE *fd;
	Object *_src;
	char * name;
	int port;
	Object *obj;
	exist = 0;
	close_program = 0;

	//Db
	/*--initializing the arguments--*/
	if (argc <4)
	{

		printf("too few arguments\n");
		exit(0);
	}

	/*--open the file to read--*/
	fd = fopen(argv[1], "r");//open file

	if (fd == NULL)
	{
		perror(argv[1]);
		fclose(fd);
		exit(EXIT_FAILURE);
	}

	src = argv[2];
	loop = atoi(argv[3]);
	n = getline(&line, &size, fd); //read line by line
	size = 0;
	num_of_edges = atoi(line);//read number of edges
	num = num_of_edges;
	//create the table to use in future
	table = createTable(num_of_edges,STR_TYPE, num_of_edges);


	//read and initialize all routers
	for (i = 0; i < num_of_edges; i++)
	{
		size = 0;
		n = getline(&line, &size, fd);
		t_line = malloc(n + 1);
		strcpy(t_line, line);
		token = strtok(line, " ");
		name = token;
		obj = createObject(STR_TYPE,name);//create one
		token = strtok(NULL, " ");
		size = 0;
		obj->ip = token;
		token = strtok(NULL, " ");
		port = atoi(token);
		obj->port = port;
		//create new entry in the table
		insertObject(table, obj, NULL,(table->d) * strHashFun(name, table->OriginSize));
	}

	//read all edges
	while ((n = getline(&line, &size, fd)) >0)
	{
		strncpy(t_line, line, n);
		token = strtok(t_line, " ");
		token2 = strtok(NULL, " ");
		token3 = strtok(NULL, " ");

		//add both to the  hash
		if(strcmp(src,token)==0||strcmp(src,token2)==0)
		{
			add(table, token, token2, atoi(token3));
			add(table, token2, token, atoi(token3));
			exist = 1;
		}
	}

	if (exist==0)
	{
		printf("no source\n");
		goto end;
	}

	free(t_line);

	//create the threads
	_src = search(table, src);
	n = countList(_src);

	for(i=0; i<table->curr_size; i++) //from i: 1 to v-s
	{
		if(table->arr[i])//if it exist
		{
			table->arr[i]->weight=INT_MAX; //d[v]=infinity
			table->arr[i]->_rec = -1;
		}
	}

	findShortestPath(table,src);
	_src->weight = 0;
	printTable(table);
	create_threads(n-1);
	printf("after create_threads\n");
end:
	freeTable(table);
	fclose(fd);
	return 0;
}
void create_threads(int n)
{
	pthread_t threads[2*n+1];
	int i, error;
	void *v;

	sockets = malloc(sizeof(int)*(2*n));

	for (i = 0; i < 2*n; i++)
	{
		v=&i;
		//create new thread
		//sender is odd , receiver is even
		error = pthread_create(threads + i, NULL, thread_op, v);
	}

	error = pthread_create(threads + 2*n, NULL, calculate, NULL);

	for (i = 0; i <=2*n; i++)
	{
		error = pthread_join(threads[i], NULL);
		printf("Joining..%d\n",i);

		if(error!=0)
			perror("failed creating a thread:");
	}

	// free(sockets);
	return;
}

//calculate the ascii sum of the name
int calc_name_ascii(char const *name)
{
	int i,sum = 0;

	for (i = 0; i < strlen(name); i++)
	{
		sum = sum+(int)name[i];
	}

	return sum;
}

void* thread_op(void *count)
{
	int sockfd,newsockfd,port, n,k;
	socklen_t clilen;
	struct sockaddr_in serv_addr,cli_addr;
	struct hostent *server;
	int i,iter, num;
	char *ip, *name;
	Object *obj;
	int ret_val = 0;
	int buf[num_of_edges+1];
	int t_buf[num_of_edges+1];

	for(i=0; i<=num_of_edges; i++)
	{
		t_buf[i] = 0;
	}

	if (!count)
	{
		perror("No Input\n");
		pthread_exit(&ret_val);
	}

	num = *(int*)count;
	i = num % 2;
	iter = num/2;
	//find the neighbor
	pthread_mutex_lock(&mutex);
	obj = search(table, (void*)src);
	pthread_mutex_unlock(&mutex);

	if(!obj)
	{
		perror("no object\n");
		pthread_exit(&ret_val);
	}

	//open socket
	sockfd= socket(AF_INET,SOCK_STREAM,0);

	if (sockfd<0)
	{
		perror("error opening socket");
		pthread_exit(&ret_val);
	}

	sockets[num] = sockfd;
	bzero((char *) &serv_addr,sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;



	if (i == 0) // case receiver
	{
		//open port to receive

		//try to own the mutex
		//initializing all setting to defaults
		name = findMyName(iter);// get the neighbor's name
		port =obj->port +calc_name_ascii(name);

		serv_addr.sin_addr.s_addr= INADDR_ANY;
		serv_addr.sin_port=htons(port);

		//try to bind the connection
		if (bind(sockfd, (struct sockaddr *)&serv_addr,sizeof(serv_addr))<0)
		{
			perror("ERROR on Bind");
			goto end;
		}


		// wait for data
		listen(sockfd,5);

		//know your client
		clilen=sizeof(cli_addr);
		//create sockets for him
		newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);

		if (newsockfd < 0)
		{
			perror("ERROR on accept");
			goto end;
		}

		while(close_program==0)
		{

			//read the data
			n = read(newsockfd, buf, sizeof(int)*(num_of_edges+1));

			if(n>0)
			{

				printf("received from %d {0/1} %d\n",port, buf[0]);

				obj = search(table, (void*)src);
				obj->_rec = buf[0];

				for (i=0; i<=num_of_edges; i++)
				{
					printf("[%d] ",buf[i]);
				}

				printf("\n");

				if (buf[0]==1)
				{
					pthread_mutex_lock(&mutex);
					updateDV(buf, name);
					pthread_mutex_unlock(&mutex);
				}

				if(all_sent()==1)
				{
					pthread_mutex_unlock(&mutex);
					pthread_cond_signal(&calc);
				}
			}

		}

		printf("finishing rec No.%d\n",iter);
		close(newsockfd);
		goto end;
	}

	else if (i == 1)//case sender
	{

		//connect to the receiver port
		port =  findMyPort(iter)+ calc_name_ascii(src);// get its ports
		ip = findMyIP(iter);//get the ip
		//get server
		server = gethostbyname(ip);

		if (server == NULL)//case server isn't found
		{
			fprintf(stderr,"ERROR, no such host\n");
			goto end;
		}



		//initializing to default
		bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
		serv_addr.sin_port = htons(port);
		k= 0;

		while (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0)
		{
			//connecting to server with the same socket and port

			if (k == loop)
			{
				perror("ERROR connecting");
				perror("reached maximum tries");

				goto end;
			}

			printf(".\n");
			k++;
			sleep(2);
		}

		buf[0] =1;
		k=1;

		for(i = 0; i<table->curr_size; i++)
		{
			if(table->arr[i])
			{
				buf[k] = table->arr[i]->weight;
				k++;
			}
		}

		printf("sending to  %d\n",port);

		for(i=0; i<=num_of_edges; i++)
			printf("[%d] ",buf[i]);

		printf(" \n");

		//keep for next iteration
		for (i=1; i<=num_of_edges; i++)
		{
			t_buf[i]=buf[i];
		}

		n = write(sockfd, buf,sizeof(int)*(num_of_edges+1));

		while(close_program==0)
		{
			pthread_cond_wait(&sender, &mutex);

			if(calc_finished==1)
				goto end;

			if(comp(buf,t_buf) ==0)// if the DV changed
			{
				buf[0] =1;
				k=1;
				printf("DV Changed\n");

				for(i = 0; i<table->curr_size; i++)
				{
					if(table->arr[i])
					{
						buf[k] = table->arr[i]->weight;
						k++;
					}
				}


				//keep for next iteration
				for (i=1; i<=num_of_edges; i++)
				{
					t_buf[i]=buf[i];
				}
			}
			else
			{
				printf("SENDER: no change\n");
				buf[0] = 0;
			}

			printf("Sending to %d  \n",port);

			for(i=0; i<=num_of_edges; i++)
				printf("[%d] ",buf[i]);

			printf("\n");

			n = write(sockfd, buf,sizeof(int)*(num_of_edges+1));

			if (n < 0)
			{
				perror("ERROR writing to socket");
				goto end;
			}

			pthread_mutex_lock(&mutex);
			pthread_cond_wait(&sender, &mutex);
			printf("Sender No.%d WOKE UP!\n",iter);
		}

	}

end:
	printf("thread %d is going off\n",iter);
	// close(sockfd);
	close_program = 1;
	pthread_cond_signal(&calc);
	pthread_exit(&ret_val);
}

int all_sent()
{
	int i;

	for(i=0; i<table->curr_size; i++)
	{
		if(table->arr[i])
			if (table->arr[i]->_rec==-1)
				return 0;
	}

	return 1;
}

int findMyPort(int num)
{
	int i ,ret;
	Object *obj;
	void *name;
	obj = search(table,src);

	for (i=0; i<num; i++)
	{
		obj = obj->next;
	}

	name =obj->data;
	obj = search(table,name);
	ret = obj->port;
	return ret;
}
char* findMyName(int num)
{
	int i;
	Object *obj;
	void *name;
	obj = search(table,src);

	for (i=0; i<num; i++)
	{
		obj = obj->next;
	}

	name =obj->data;
	return (char*)name;
}
char* findMyIP(int num)
{
	int i;
	Object *obj, *name;
	char *ip;
	obj = search(table,src);

	for (i=0; i<num; i++)
	{
		obj = obj->next;
	}

	name =obj->data;
	obj = search(table,name);
	ip =(char*)obj->ip;
	return ip;
}
void* calculate(void* count)
{
	int sp,i;

	while(1)
	{
		pthread_mutex_lock(&mutex);
		pthread_cond_wait(&calc, &mutex);
		printf("calculate woke up!\n");

		//lock for use
		if(close_program==1)
		{
			printf("time to go...\n");
			goto end;
		}

		if(check_all_sent_zero()==1)
		{
			printf("in calc: time to go...\n");
			toString(table,src);
			pthread_cond_signal(&sender);
			goto end;

		}

		sp =findShortestPath(table,src);
		printTable(table);

		pthread_mutex_lock(&mutex);

		for(i=0; i<table->curr_size; i++)
			table->arr[i]->_rec = -1;

		pthread_mutex_unlock(&mutex);

		if(sp== NEG_CRCLE)
		{
			printf("negative circle found");
			goto end;
		}




		//reset all received
		for (i=0; i<table->curr_size; i++)
		{
			if (table->arr[i])
				table->arr[i]->_rec = -1;
		}

		//release for use
		//call sender
		pthread_cond_signal(&sender);
	}

end:
	close_program=1;
	pthread_cond_signal(&sender);

	for(i=0; i<sizeof(sockets)/sizeof(int); i++)
	{
		printf("closing sck %d\n",i);
		close(sockets[i]);
	}

	printf("finished closing all\n");
	pthread_exit(NULL);
}
int check_all_sent_zero()
{
	int i;

	for(i=0; i<table->curr_size; i++)
	{
		if (table->arr[i])
			if(strcmp((char*)table->arr[i]->data,src)!=0)
				if (table->arr[i]->_rec!=0)
					return 0;
	}

	return 1;
}
void buffer_ini(char *buf[])
{
	int i, k;
	k=0;

	for(i = 0; i<table->curr_size; i++)
	{
		if(table->arr[i])
		{
			*buf[k] = (char)table->arr[i]->weight;
			k++;
		}
	}

	return;
}

void updateDV(int buf[], char* n_name)
{
	Object *head,*temp;
	int i, found=0;
	char* name;


	head = search(table, (void*)n_name);

	for(i=1; i<=num_of_edges; i++)
	{

		temp = head->next;

		while(temp!=NULL)
		{
			name = find_by_place(i-1);

			if(strcmp((char*)temp->data,name)==0)
			{
				found = 1;
				temp->weight = buf[i];
			}

			temp = temp->next;
		}

		if(found==0)
		{
			add(table,n_name,name,buf[i]);
		}

		found = 0;
	}

	/*  while (temp!=NULL)
	    {
	        temp->weight =buf[i];
	        temp = temp->next;
	        i++;
	    }
	*/
	return;
}
char* find_by_place(int place)
{
	int i = 0,k;

	for(k=0; k<table->curr_size; k++)
	{
		if(table->arr[i])
		{
			if(i==place)
				return (char*)table->arr[i];

			i++;
		}
	}

	return NULL;
}
void printBuf(int buf[])
{
	int i;

	for (i=0; i<num_of_edges+1; i++)
	{
		printf("weight: %d",buf[i]);
	}

	return;
}

int comp(int arr[], int arr1[])
{
	int i;

	for (i=0; i<(sizeof(arr)/sizeof(int)); i++)
	{
		if (arr[i]!=arr1[i])
			return -1;
	}

	return 0;
}