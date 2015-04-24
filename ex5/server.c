#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netdb.h>
#define SIZE 1024
int main(int argc, char const *argv[])
{
	int sockfd,port,clilen, n,i,j;
	struct sockaddr_in serv_addr,cli_addr;
	fd_set set, active;
	int sockets;
	int max;
	char buff[SIZE+1];

	if(argc<2)
	{
		printf("Not enough arguments\n");
		exit(0);
	}

	port = atoi(argv[1]);


	//get socket
	sockfd = socket(AF_INET,SOCK_STREAM,0);

	if (sockfd <0)
		perror("ERROR opening socket");



	bzero((char *) &serv_addr,sizeof(serv_addr));
	printf("\nthe port is: %d\n",port);
	serv_addr.sin_family= AF_INET;
	serv_addr.sin_addr.s_addr= htonl(INADDR_ANY);
	serv_addr.sin_port=htons(port);

	if (bind(sockfd , (struct sockaddr*)&serv_addr,sizeof(serv_addr))<0)
		perror("ERROR on binding");

	listen(sockfd ,5);

	//initialize the set
	FD_ZERO(&active);
	FD_SET(sockfd ,&active);
	max = sockfd;

	while(1)
	{
		set = active;

		if(select(max+1, &set,NULL,NULL,NULL)>0)
		{
			// perror("On Select");


			for(i=0; i<=max; i++)
			{
				if(FD_ISSET(i,&set))
				{
					if(i==sockfd )//if the request is on original socket
					{

						clilen=sizeof(cli_addr);
						sockets  = accept(sockfd , (struct sockaddr *)&cli_addr, &clilen);

						if (sockets  < 0)
							perror("ERROR on accept");
						else
						{

							// add to set
							FD_SET(sockets ,&active);

							if(sockets>max)
								max = sockets;
						}


					}
					else
					{
						bzero(buff,SIZE+1);
						n = read(i , buff, SIZE);

						if (n < 0)
						{
							perror("ERROR reading from socket");
							exit(0);
						}
						else if (n==0)
						{
							FD_CLR(i,&active);
							close(i);
							printf("close socket  %d\n",i);
						}
						else
						{
							printf("message:%s\n", buff);


							for(j=0; j<=max; j++)
							{
								if(FD_ISSET(j,&active))
								{
									if(j!=sockfd&&j!=i)
										n = write(j,buff,n);
								}
							}

							bzero(buff,SIZE+1);

						}

					}
				}
			}

		}
	}

	close(sockfd);
	return 0;
}
