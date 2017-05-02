#include<stdio.h>
#include<string.h>
#include<pthread.h>
#include<stdlib.h>
#include<unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define BUFFSIZE 5000

//reads from socket and writes to stdout
void *writethread(void *args)
{
    int readct;
    int *sockfd = (int*) args;
    char *buff =  malloc(sizeof(char*)*BUFFSIZE);
    while(1)
    {   
        readct = read(*sockfd,buff,BUFFSIZE);
        if (readct<0)
        {
            perror("read");    
        }
        else if (readct == 0)
        {
            pthread_exit(NULL);
        }
        write(1,buff,readct);
        write(1,">>",2);
    }

}

//reads from stdin and writes to socket
void *readthread(void *args)
{   
    int *sockfd = (int*) args;
    char *buff =  malloc(sizeof(char*)*BUFFSIZE);
    int readct;
    write(1,">>",2);
    while(1)
    {  
        
        readct = read(0,buff,BUFFSIZE);
        if (readct<0)
        {
            perror("error in read");
            exit(-1);
        }
        // else if(readct == 1){ //fix readbuff == /n
        //     continue;
        // }
        write(*sockfd,buff,readct);
        //write(1,"\n",sizeof("\n"));
    }
}

int main(int argc, char *argv[])
{
    pthread_t readthreadid;
    pthread_t writethreadid;

    int sock;
    struct sockaddr_in server;
    struct hostent *hp;

    /* Create socket */
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("opening stream socket");
        exit(1);
    }
    /* Connect socket using name specified by command line. */
    server.sin_family = AF_INET;
    hp = gethostbyname(argv[1]);
    if (hp == 0) {
        fprintf(stderr, "%s: unknown host\n", argv[1]);
        exit(2);
    }
    bcopy(hp->h_addr, &server.sin_addr, hp->h_length);
    server.sin_port = htons(atoi(argv[2]));

    if (connect(sock,(struct sockaddr *) &server,sizeof(server)) < 0) {
        perror("connecting stream socket");
        exit(1);
    }
    int tcreateret1 = pthread_create(&(readthreadid), NULL, &readthread, (void*)&sock);
    int tcreateret2 = pthread_create(&(writethreadid), NULL, &writethread, (void*)&sock);
    if (tcreateret1 < 0 || tcreateret2 < 0) {
        perror("pthread_create");
        return -1;
    }
    pthread_join(readthreadid, (NULL));
    pthread_join(writethreadid, (NULL));
  close(sock);
  return 0;
}

//tokenization
char **token(char *line)
{
  char **input = malloc(sizeof(char*)*BUFFSIZE);
  char *token = malloc(sizeof(char*)*BUFFSIZE);
  int position=0;
  token= strtok(line," ");
  while(token!=NULL)
  {
	input[position]=token;
	position++;
	token = strtok (NULL," ");
  }
  input[position]='\0';
  return input;
}
  