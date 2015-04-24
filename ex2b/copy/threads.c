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
void create_threads(int n)
{
	pthread_t threads[2*n+1];
	int i, error;
	void *v, *ret;

	for (i = 0; i < 2*n; i++)
	{
		v=&i;
		//create new thread
		//sender is odd , receiver is even
		error = pthread_create(threads + i, NULL, thread_op, v);

	}

	error = pthread_create(threads + 2*n, NULL, calculate, NULL);



	for (i = 0; i < 2*n; i++)
	{
		error = pthread_join(threads[i], &ret);
		printf("Joining..%d\n",i);

		if(error!=0)
			perror("failed creating a thread:");
	}

	error = pthread_join(threads[2*n], &ret);
	return;
}

void* thread_op(void *count)
{
	int sockfd,newsockfd,port, n,k;
	socklen_t clilen;
	struct sockaddr_in serv_addr,cli_addr;
	struct hostent *server;
	int i,iter, num, mod;
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
	mod = num % 2;
	iter = num/2;
	//find the neighbor
	obj = search(table, (void*)src);

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

	bzero((char *) &serv_addr,sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;



	if (mod == 0) // case receiver
	{
		//open port to receive

		// initializing all setting to defaults
		name = findMyName(iter);// get the neighbor's name
		serv_addr.sin_addr.s_addr= INADDR_ANY;
		port =obj->port +calc_name_ascii(name);
		serv_addr.sin_port=htons(port);

		//try to bind the connection
		if(bind(sockfd, (struct sockaddr *)&serv_addr,sizeof(serv_addr))<0)
		{
			printf("BIND to port %d\n", port);
			perror("Bind");
			// exit(0);
			// pthread_mutex_unlock(&calc_ended_mutex);
			// pthread_cond_signal(&sender_quit);
			// pthread_cond_signal(&receive);
			calc_finished = 1;
			flag = 1;
		}


		// wait for data
		listen(sockfd,5);
		//know your client
		clilen=sizeof(cli_addr);

		newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);


		if (newsockfd < 0)
		{
			perror("ERROR on accept");

			close(sockfd);
			pthread_exit(&ret_val);
		}

		while(flag==0)
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
					// pthread_mutex_lock(&mutex);
					updateDV(buf, name);
					// pthread_mutex_unlock(&mutex);
				}

				if(all_sent()==1)
					pthread_cond_signal(&calc);
			}

			printf("receiver No.%d SLEEPING!\n",iter);
			// pthread_cond_wait(&receive,&mutex);
			printf("receiver No.%d WOKE UP!\n",iter);
		}


		// pthread_cond_wait(&receive,&calc_ended_mutex);
		close(sockfd);
		close(newsockfd);
		pthread_exit(&ret_val);
	}

	else if (mod== 1)//case sender
	{
		// pthread_mutex_lock(&mutex);
		//connect to the receiver port
		port =  findMyPort(iter)+ calc_name_ascii(src);// get its ports
		ip = findMyIP(iter);//get the ip
		//get server
		server = gethostbyname(ip);

		if (server == NULL)//case server isn't found
		{
			fprintf(stderr,"ERROR, no such host\n");
			pthread_exit(&ret_val);
		}



		//initializing to default
		bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
		serv_addr.sin_port = htons(port);
		k= 0;

		printf("\nthread No.%d connecting to port %d\n",num,port);

		while (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0)
		{
			//connecting to server with the same socket and port

			sleep(1);

			if (k == loop)
			{
				perror("ERROR connecting");
				printf("thread %d reached maximum tries\n",num);
				flag = 1;
				calc_finished= 1;
				// pthread_cond_signal(&receive);
				close(sockfd);
				pthread_exit(&ret_val);
				return NULL;
			}

			printf(".\n");
			k++;
		}

		printf("\n***********************sending to port: %d****************************\n",port);

		while(flag==0)
		{
			if(calc_finished==1)
				pthread_exit(&ret_val);

			if(comp(buf,t_buf) ==0)// if the DV changed
			{
				printf("Sending to %d , DV changed	\n",port);
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

				for(i=0; i<=num_of_edges; i++)
					printf("[%d] ",buf[i]);

				printf(" \n");

				//keep for next iteration
				for (i=1; i<=num_of_edges; i++)
				{
					t_buf[i]=buf[i];
				}
			}
			else
			{
				buf[0] = 0;
				printf("SENDER: no change\n");
			}

			n = write(sockfd, buf,sizeof(int)*(num_of_edges+1));

			if (n < 0)
			{
				perror("ERROR writing to socket");
				pthread_exit(&ret_val);
			}

			// pthread_mutex_unlock(&calc_ended_mutex);
			changed = 0;
			// pthread_cond_wait(&sender, &calc_ended_mutex);
			printf("Sender No.%d WOKE UP!\n",iter);
		}

		// pthread_cond_wait(&sender_quit, &calc_ended_mutex);
		close(sockfd);
		pthread_exit(&ret_val);
	}

	printf("\nthread No.%d quitting\n",num);
	close(sockfd);
	pthread_exit(&ret_val);
	return NULL;
}
