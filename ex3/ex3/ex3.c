#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>

#define FAILURE -2
#define SIZE 10
char* read_socket(int sockfd,char * postfix, int normal_read);
int write_socket(FILE* fd,int sockfd, char *prefix);
int main(int argc, char const *argv[])
{
  const char *file = NULL, *ch = NULL;
  int count=0, len = 0;
  int normal_read =1;
  FILE* fd = NULL;
  int sockfd, n, port=25;
  struct sockaddr_in serv_addr;
  struct hostent *server = NULL;
  char *buffer = NULL,*postfix, *temp;
  int flag = 1;
  /*check for arguments*/
  if(argc==4)//case all exist
  {
    file = argv[1];//the file
    ch = argv[2];//the server
    port = atoi(argv[3]);
}
  else if (argc==3)//case port is missing
  {
    file = argv[1];
    ch = argv[2];
}
  else if(argc<3)//none at all
  {
    printf("Not Enough Arguments\n");
    exit(EXIT_FAILURE);
}

    /*open the file to read*/
    fd = fopen(file, "r");//open file
    if (fd == NULL)
    {
      perror(file);
      fclose(fd);
      exit(EXIT_FAILURE);
  }

    //Opens the main socket
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0)
  {
      perror("ERROR opening socket");
      fclose(fd);
      exit(EXIT_FAILURE);
  }

    /*get the server*/
  server = gethostbyname(ch);
  if (server == NULL)
  {
      fprintf(stderr,"ERROR, no such host\n");
      fclose(fd);
      exit(0);
  }
  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;

  bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);

  serv_addr.sin_port = htons(port);

        //connecting to server with the same socket and port
  if (connect(sockfd,(struct sockaddr*)&serv_addr,sizeof(serv_addr)) < 0)
      perror("ERROR connecting");


    // get first line from server
  temp = read_socket(sockfd,"\n",normal_read);
  if(temp)
  {
    postfix = malloc(sizeof(temp)+1);
    strcpy(postfix,temp);
    free(temp);
    }
    //init
count = 0;
n = 0;
    while(1)//until we finish
    {
      switch(count)
      {
        case 0://say helo
        len = strlen("helo ")+strlen(argv[2])+1;
        buffer = malloc(len+1);
        bzero(buffer,len);
        strcpy(buffer,"helo ");
        strcat(buffer,ch);
        strcat(buffer,"\n");
        printf("Client: %s",buffer);
        n = write(sockfd,buffer,len);
        temp= read_socket(sockfd,"\n",!normal_read);
        free(buffer);//cleanup
        break;

        case 1:
        n = write_socket(fd,sockfd,"MAIL FROM: ");
        temp= read_socket(sockfd,postfix,!normal_read);
        break;

        case 2:
        n = write_socket(fd,sockfd,"RCPT TO: ");
        temp =read_socket(sockfd,postfix,!normal_read);
        break;

        case 3:
        buffer = "DATA\n";
        n = write(sockfd, buffer, strlen(buffer));
        printf("Client: %s",buffer);
        temp =read_socket(sockfd,"\n",!normal_read);
        break;
    }
    if(count>3)
    {
        n = write_socket(fd,sockfd,NULL);
    }
    if(n==-1)
        break;
    if(temp==NULL)
      flag = 0;
  count++;
}
if(flag)
{

    buffer  = "\r\n.\r\n";
    n = write(sockfd,buffer,strlen(buffer));
    printf("Client: %s",buffer);
    read_socket(sockfd,postfix,!normal_read);
    buffer  = "QUIT\n";
    printf("Client: %s",buffer);
    n = write(sockfd,buffer,strlen(buffer));
}
close(sockfd);
fclose(fd);
free(postfix);
return 1;
}

char* read_socket(int sockfd,char *postfix, int normal_read)
{
    int n = 0;
    char *buffer = NULL ,*buf = NULL,*temp = NULL;
    char *post, *ret = NULL;
    if (!postfix)
      return NULL;

  buf = malloc(SIZE+1);
  buffer = malloc(SIZE+1);
  bzero(buffer,SIZE+1);
  do
  {
    bzero(buf,SIZE+1);//keep \0 at the end
    n = read(sockfd,buf,SIZE);//read from server
    if(n<0)
    {
      free(buf);
      free(buffer);
      return NULL;
  }
    else//case all buffer is full
      buffer = realloc(buffer,strlen(buffer)+n+1);
  if(buffer==NULL)
      break;
  strcat(buffer,buf);
}while((ret =strstr(buffer,postfix))==NULL);
free(buf);
post= postfix;
if (normal_read)
{
    n= 1;
    temp = NULL;
    temp = strstr(buffer,"- ");
    temp = temp+2;
    while(temp[n]!='\n'){n++;}
    post = malloc(n+1);
    bzero(post,n);
    strncpy(post,temp,n);
}
printf("Server: %s",buffer);
if(strstr(buffer,"421")==NULL&&buffer[0]>'3')
{
    free(buffer);
    return NULL;
}
free(buffer);
return post;
}

int write_socket(FILE* fd,int sockfd, char *prefix)
{
  int n= SIZE;
  size_t size=0;
  char *buffer = NULL;
  char *buffer2 = NULL;
  if(fd==NULL)
    return FAILURE;
if(!prefix)
    n=getline(&buffer,&size,fd);
if(n<=0)
{
    free(buffer);
    return -1;
}
else if(prefix)
{
    n=getline(&buffer2,&size,fd);
    if(n<=0)
    {
      free(buffer2);
      return -1;
  }
  buffer = malloc(strlen(prefix)+n+1);
  buffer= strcpy(buffer,prefix);
  buffer= strcat(buffer,buffer2);
  free(buffer2);
}
if(strlen(buffer)>=0)
{
    n = write(sockfd, buffer, strlen(buffer));
    if (n < 0)
      perror("ERROR writing to socket");
  printf("Client: %s",buffer);
}
free(buffer);
return n;
}
