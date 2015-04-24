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


void create_threads(int n);//create the threads
void *thread_sender(void *count);//decides if it to send or receive
void *thread_receive(void *count);//decides if it to send or receive
void *calculate(void *count);//the BF
void updateDV(int buf[], char *n_name);//
int all_sent();//check is all have sent their DV
int check_all_sent_zero();//check is all have sent their DV with 0
int comp(int arr[], int arr1[]);
char *findMyIP(int num);//to find ones IP
char *findMyName(int num);//to find ones name
int calc_name_ascii(char const *name);//convert the name to ascii
int findMyPort(int num);//finds the port
char *find_by_place(int place);//returns its name according to the place on the array
int loop;//tries aloud
int close_program;//he flag
Table *table;//the hash
const char *src;//source name
int num_of_edges;//number of edges
int *sockets;//the sockets

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t calc = PTHREAD_COND_INITIALIZER;
pthread_cond_t sender = PTHREAD_COND_INITIALIZER;


int main(int argc, char const *argv[])
{
	int exist;
	int n, i;
	size_t size = 0;
	char     *line, *t_line, *token, *token2, *token3;
	FILE *fd;
	Object *_src;
	char *name, *ip;
	int port;
	Object *obj;
	exist = 0;
	close_program = 0;

	//Db
	/*--initializing the arguments--*/
	if (argc < 4)
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
	//create the table to use in future
	table = createTable(num_of_edges, STR_TYPE, num_of_edges);


	//read and initialize all routers
	for (i = 0; i < num_of_edges; i++)
	{
		n = getline(&line, &size, fd);
		t_line = malloc(n + 1);
		strcpy(t_line, line);
		token = strtok(line, " ");
		name = token;
		token = strtok(NULL, " ");
		size = 0;
		ip = token;
		token = strtok(NULL, " ");
		port = atoi(token);
		//create new entry in the table
		addObject(table, name, port, ip);

		if (strcmp(src, name) == 0)
			exist = 1;
	}

	if (exist == 0)
	{
		printf("no source\n");
		goto end;
	}

	exist = 0;

	//read all edges
	while (getline(&line, &size, fd) != -1)
	{
		size = 0;
		strncpy(t_line, line, n);
		token = strtok(t_line, " ");//fist vertex
		token2 = strtok(NULL, " ");//second vertex
		token3 = strtok(NULL, " ");//edge

		//add both to the  hash only in the source is one of them
		if (strcmp(src, token) == 0 || strcmp(src, token2) == 0)
		{
			//add the edge for the two vertexes
			add(table, token, token2, atoi(token3));
			add(table, token2, token, atoi(token3));
			exist = 1;
		}
	}

	if (exist == 0)
	{
		printf("no neighbors\n");
		goto end;
	}

	free(t_line);

	//create the threads
	_src = search(table, src);//find the source
	n = countList(_src);//count the number of neighbors

	/*update all to infinity*/
	for (i = 0; i < table->curr_size; i++) //from i: 1 to v-s
	{
		if (table->arr[i]) //if it exist
		{
			table->arr[i]->weight = INT_MAX; //d[v] = infinity
		}
	}

	obj = _src;

	/*set all to not sent*/
	for (i = 0; i < n; i++)
	{
		obj = obj->next;

		if (obj)
			obj->_rec = -1;
	}

	// set sourde to 0
	_src->weight = 0;
	//calculate for first time
	findShortestPath(table, src);
	//create the thread
	create_threads(n - 1);
end:
	// cleanup
	freeTable(table);
	fclose(fd);
	return 0;
}


/*this function creates the threads
  and waits for them to finish*/
void create_threads(int n)
{
	pthread_t r_threads[n];//receivers array
	pthread_t s_threads[n];//senders array
	pthread_t calc;
	int arr[n];//to save the data
	int i, error;
	void *v;
	close_program = 0;//set the flag off
	sockets = malloc(sizeof(int) * 2 * n);// to be able to close the sockets from the calculator

	error = pthread_create(&calc, NULL, calculate, NULL);

	if (error != 0)
	{
		perror("failed creating a thread:");
	}

	for (i = 0; i < n; i++)
	{
		arr[i] = i;
		v = &arr[i] ;
		//create new threads
		error = pthread_create(&s_threads[i], NULL, thread_sender, v);

		if (error != 0)
		{
			perror("failed creating a thread:");
		}

		error = pthread_create(&r_threads[i], NULL, thread_receive, v);

		if (error != 0)
		{
			perror("failed creating a thread:");
		}
	}

	for (i = 0; i < n; i++)
	{
		error = pthread_join(s_threads[i], NULL);
		error = pthread_join(r_threads[i], NULL);

		if (error != 0)
			perror("failed creating a thread:");
	}

	error = pthread_join(calc, NULL);

	return;
}

//calculate the ASCII sum of the name
int calc_name_ascii(char const *name)
{
	int i, sum = 0;

	for (i = 0; i < strlen(name); i++)
	{
		sum = sum + (int)name[i];
	}

	return sum;
}


/*sends the DV*/
void *thread_sender(void *count)
{
	int sockfd, n, k, i, port;
	struct sockaddr_in serv_addr;
	struct hostent *server;
	char *ip;
	int *buf;
	int *t_buf;
	int num;
	num = *(int *)count;
	buf = malloc(sizeof(int) * (num_of_edges + 1));
	t_buf = malloc(sizeof(int) * (num_of_edges + 1));

	//set the copy buff to -1 to first iteration will give "DV changed"
	for (i = 0; i <= num_of_edges; i++)
	{
		t_buf[i] = -1;
	}

	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	if (sockfd < 0)
	{
		perror("error opening socket");
		pthread_exit(NULL);
	}

	pthread_mutex_lock(&mutex);
	ip = findMyIP(num);
	server = gethostbyname(ip);
	//connect to the receiver port
	port =  findMyPort(num) + calc_name_ascii(src); // get its ports
	pthread_mutex_unlock(&mutex);

	if (server == NULL)//case server isn't found
	{
		fprintf(stderr, "ERROR, no such host\n");
		goto end;
	}

	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;

	bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);

	serv_addr.sin_port = htons(port);

	//connecting to server with the same socket and port
	k = 0;

	while (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
	{
		sleep(1);

		if (k == loop)
		{
			perror("In Connect");
			perror("Reached Maximum Tries");
			goto end;
		}

		printf(".\n");
		k++;
	}

	//first time send the data
	k = 1;
	pthread_mutex_lock(&mutex);

	for (i = 0; i < table->curr_size; i++)
	{
		if (table->arr[i])
		{
			buf[k] = table->arr[i]->weight;
			k++;
		}
	}

	pthread_mutex_unlock(&mutex);

	if (comp(buf, t_buf) == -1) // if the DV changed
	{
		buf[0] = 1;
		k = 1;

	}
	else
	{
		buf[0] = 0;

	}

	//copy for next time
	for (i = 1; i <= num_of_edges; i++)
	{
		t_buf[i] = buf[i];
	}

	n = write(sockfd, buf, sizeof(int) * (num_of_edges + 1));

	if (n < 0)
	{
		perror("ERROR writing to socket");
		goto end;
	}

	pthread_mutex_lock(&mutex);

	while (close_program == 0)
	{
		pthread_cond_wait(&sender, &mutex);
		k = 1;

		for (i = 0; i < table->curr_size; i++)
		{
			if (table->arr[i])
			{
				buf[k] = table->arr[i]->weight;
				k++;
			}
		}



		if (comp(buf, t_buf) == -1) // if the DV changed
		{
			buf[0] = 1;
			k = 1;

		}
		else
		{
			buf[0] = 0;

		}


		for (i = 1; i <= num_of_edges; i++)
		{
			t_buf[i] = buf[i];
		}

		pthread_mutex_unlock(&mutex);
		n = write(sockfd, buf, sizeof(int) * (num_of_edges + 1));

		if (n < 0)
		{
			perror("ERROR writing to socket");
			goto end;
		}
	}

end:
	pthread_mutex_unlock(&mutex);
	close(sockfd);
	return NULL;
}

//receive the data and saves it
void *thread_receive(void *count)
{
	int sockfd, newsockfd, portno, n;
	socklen_t clilen;

	int *buf;
	int num = *(int *)count;
	struct sockaddr_in serv_addr, cli_addr;
	Object *obj;
	char *name;

	buf = malloc(sizeof(int) * (num_of_edges + 1));

	pthread_mutex_lock(&mutex);
	name = findMyName(num);// get the neighbor's name


	//find the neighbor
	obj = search(table, (void *)src);

	pthread_mutex_unlock(&mutex);

	if (!obj)
	{
		printf("no object\n");
		return NULL;
	}

	//calculate the port
	portno = obj->port + calc_name_ascii(name);

	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	if (sockfd < 0)
	{
		perror("error opening socket");
		return NULL;
	}

	sockets[num] = sockfd;//save to be able to close after

	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);


	if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		perror("ERROR on Bind");
		return NULL;
	}

	listen(sockfd, 5);
	clilen = sizeof(cli_addr);
	newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);

	if (newsockfd < 0)
	{
		perror("ERROR on accept");
		return NULL;
	}

	sockets[sizeof(sockets) / sizeof(int) - num - 1] = newsockfd;


	pthread_mutex_lock(&mutex);

	while (close_program == 0)
	{

		pthread_mutex_unlock(&mutex);
		//read the data
		n = read(newsockfd, buf, sizeof(int) * (num_of_edges + 1));

		if (n > 0)
		{
			pthread_mutex_lock(&mutex);
			obj = search(table, (void *)src);//find the source
			obj = obj->next;

			while (obj)//copy the state to the vertex
			{
				if (strcmp((char *)src, (char *)name) == 0)
				{
					obj->_rec = buf[0];
					break;
				}

				obj = obj->next;
			}


			//update the new DV in the hash
			updateDV(buf, name);

			//check if all sent
			if (all_sent() == 1)
			{
				pthread_cond_signal(&calc);
			}

			pthread_mutex_unlock(&mutex);


		}
		else if (n <= 0)
			goto end;

	}

end:
	pthread_mutex_unlock(&mutex);
	close(sockfd);
	close(newsockfd);
	return NULL;
}

//runs on the hash
//check for each neighbor of the source if got from him
int all_sent()
{
	int i;
	Object *head, *temp;
	head = search(table, src);
	temp = head;

	for (i = 0; i < countList(head); i++)
	{
		temp = temp ->next;

		if (temp)
		{
			if (temp->_rec == -1)
				return 0;
		}
	}

	return 1;
}

//runs on the hash
//check for each neighbor of the source if got 0 from him
int check_all_sent_zero()
{
	int i;
	Object *temp;
	Object *head = search(table, src);
	temp = head;

	for (i = 0; i < countList(head); i++)
	{
		temp = temp->next;

		if (temp)
			if (temp->_rec != 0)
				return 0;
	}

	return 1;
}

//finds the vertex' port
int findMyPort(int num)
{
	int i ;
	Object *obj;
	void *name;
	obj = search(table, src);

	for (i = 0; i <= num; i++)
	{
		obj = obj->next;

	}

	name = obj->data;
	obj = search(table, name);
	return obj->port;
}

//finds the vertex' name
char *findMyName(int num)
{
	int i;
	Object *obj;

	obj = search(table, src);

	for (i = 0; i <= num; i++)
	{
		obj = obj->next;
	}

	return (char *)obj->data;
}

//finds the vertex' IP
char *findMyIP(int num)
{
	int i;
	Object *obj, *name;
	char *ip;
	obj = search(table, src);

	for (i = 0; i <= num; i++)
	{
		obj = obj->next;

	}

	name = obj->data;
	obj = search(table, name);
	ip = (char *)obj->ip;

	return ip;
}



//calculator
void *calculate(void *count)
{
	int sp, i;
	Object *head;
	pthread_mutex_lock(&mutex);

	while (close_program == 0)
	{
		pthread_cond_wait(&calc, &mutex);
		sp = findShortestPath(table, src);//Bellman-Ford


		if (sp == NEG_CRCLE)
		{
			printf("negative circle found");
			goto end;
		}

		pthread_mutex_unlock(&mutex);
		pthread_cond_broadcast(&sender);//wake senders
		pthread_mutex_lock(&mutex);

		if (check_all_sent_zero() == 1)
		{
			close_program = 1;
			toString(table, src);
			pthread_cond_broadcast(&sender);


			//close all receivers' socket
			for (i = 0; i < (sizeof(sockets) / sizeof(int)); i++)
			{
				close(sockets[i]);
			}

			goto end;
		}

		//reset all received
		head = search(table, src);

		for (i = 0; i < countList(head); i++)//set all back to not received
		{
			head = head->next;

			if (head)
				head->_rec = -1;
		}

		pthread_mutex_unlock(&mutex);
	}


end:
	pthread_mutex_unlock(&mutex);
	return NULL;
}

//updates the hash with the DV
// add/re-set the given vertex' edges and neighbors
void updateDV(int buf[], char *n_name)
{
	Object *head, *temp;
	int i, found = 0;
	char *name;

	head = search(table, src);
	temp = head;

	for (i = 0; i < countList(head); i++)
	{
		temp = temp->next;

		if (temp)
			if (strcmp(n_name, (char *)temp->data) == 0)
				temp->_rec = buf[0];
	}

	if (buf[0] == 0)
		return;

	head = search(table, (void *)n_name);

	for (i = 1; i <= num_of_edges; i++) //run on the DV
	{

		name = find_by_place(i - 1); //find which neighbor is it

		if (!head->next)
		{
			if (buf[i] != INT_MAX)
				add(table, n_name, name, buf[i]);
		}

		temp = head->next;


		while (temp != NULL)
		{

			if (strcmp((char *)temp->data, name) == 0)
			{
				found = 1;

				if (buf[i] != INT_MAX)
					temp->weight = buf[i];

			}

			temp = temp->next;
		}

		if (found == 0)
			if (buf[i] != INT_MAX)
				add(table, n_name, name, buf[i]);

		found = 0;
	}

	return;
}

//finds a vertex by given index
char *find_by_place(int place)
{
	int i = 0, k;

	for (k = 0; k < table->curr_size; k++)
	{
		if (table->arr[k])
		{
			if (i == place)
			{
				return (char *)table->arr[k]->data;
			}

			i++;
		}
	}

	return NULL;
}

//compares the buffers
//returns -1 if not compared
//0 if yes
int comp(int arr[], int arr1[])
{
	int i;

	for (i = 1; i < (sizeof(arr) / sizeof(int)); i++)
	{
		if (arr[i] != arr1[i])
			return -1;
	}

	return 0;
}
