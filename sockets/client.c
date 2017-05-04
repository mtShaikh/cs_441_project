#include<stdio.h>
#include<string.h>
#include<pthread.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define BUFFSIZE 5000
#define PORT 3490
char **token(char *line);
int serverconnect(char *host, char *port);
static int serverstatus = 2;

//reads from socket and writes to stdout
void *writethread(void *args)
{
	int readct;
	int *sockfd = (int*) args;
	char *buffer =  malloc(sizeof(char*)*BUFFSIZE);
	while(1)
	{   
		if (serverstatus == 0)
		{
			close(*sockfd);
			pthread_exit(NULL);
		}
		readct = read(*sockfd,buffer,BUFFSIZE);
		if (readct<0)
		{
			perror("read");
			exit(-1);    
		}
		else if (readct == 0)
		{
			write(1,"\ndisconnected from server\n",sizeof("\ndisconnected from server\n"));
			serverstatus = 0;
			close(*sockfd);
			pthread_exit(NULL);
		}
		write(1,"\b",2);
		write(1,"\b",2);
		write(1,buffer,readct);
		write(1,"$:",2);
	}

}

//reads from stdin and writes to socket
void *readthread(void *args)
{   
	int *sockfd = (int*) args;
	char *buffer =  malloc(sizeof(char*)*BUFFSIZE);
	char **line;
	int readct;
	while(1)
	{  
		if (serverstatus == 0)
		{
			close(*sockfd);
			pthread_exit(NULL);
		}
		write(1,"$:",2);
		readct = read(0,buffer,BUFFSIZE);
		char *inputline = malloc(strlen(buffer));
		strcpy(inputline,buffer);
		if (readct<0)
		{
			perror("error in read");
			exit(-1);
		}
		else if (readct == 0)
		{
			write(1,"disconnected from server\n",sizeof("disconnected from server\n"));
			close(*sockfd);
			pthread_exit(NULL);
		}
		line = token(buffer);
		char *tokenfirst = line[0];
	  	sscanf(*line,"%s",tokenfirst);
		if(strcasecmp(tokenfirst,"exit") == 0)
		{
			write(1, "exiting\n", sizeof("exiting\n"));
			free(buffer);
			close(*sockfd);
			exit(0);
		}
		else if(strcasecmp(tokenfirst,"disconnect") == 0)
		{
			write(1, "disconnecting from server\n", sizeof("disconnecting from server\n"));
			serverstatus = 0;
			close(*sockfd);
			pthread_exit(NULL);
		}
		else if(strcasecmp(tokenfirst,"\n")==0)
		{
			continue;
		}
		else
		{	//printf("aa%s\n",inputline );
			int writect = write(*sockfd,inputline,readct);
			if (writect < 0)
			{
				// write(1,"disconnected from server\n",sizeof("disconnected from server\n"));
				// close(*sockfd);
				// pthread_exit(NULL);	
			}
			// else if (writect==0)
			// {
				
			// }
		}
	}
}

int main(int argc, char *argv[])
{
	
	char *buffer =  malloc(sizeof(char*)*BUFFSIZE);
	char **line;
	int readct;
	int sock;
	pthread_t readthreadid;
	pthread_t writethreadid;

	while(1)
	{
		write(1,">>",2);
		readct = read(0,buffer,BUFFSIZE);
		if (readct<0)
		{
			perror("error in read");
			exit(-1);
		}
		line = token(buffer);
		char *tokenfirst = line[0];
	  	sscanf(*line,"%s",tokenfirst);
		if(strcasecmp(tokenfirst,"exit") == 0)
		{
			write(1, "exiting\n", sizeof("exiting\n"));
			free(buffer);
			exit(0);
		}
		else if(strcasecmp(tokenfirst,"connect") == 0)
		{
			sock = serverconnect(line[1], line[2]);
			if (sock < 0)
			{
				continue;
			}
			serverstatus = 1;
			int tcreateret1 = pthread_create(&(readthreadid), NULL, &readthread, (void*)&sock);
			int tcreateret2 = pthread_create(&(writethreadid), NULL, &writethread, (void*)&sock);
			if (tcreateret1 < 0 || tcreateret2 < 0) 
			{
				perror("pthread_create");
				exit(1);
			}
			pthread_join(readthreadid, (NULL));
			if (serverstatus == 0)
			{
				pthread_cancel(writethreadid);
			}
			pthread_join(writethreadid, (NULL));
			close(sock);
			continue;
		}
		else if(strcasecmp(tokenfirst,"\n")==0)
		{
			continue;
		}
		else
		{
			continue;
		}
	}
	return 0;
}

int serverconnect(char *host, char *port)
{
	int sock;
	struct sockaddr_in server;
	struct hostent *hp;

	/* Create socket */
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0) 
	{
		perror("opening stream socket");
		exit(1);
	}
	server.sin_family = AF_INET;
	hp = gethostbyname(host);
	if (hp == 0) 
	{
		char out[64];
		int pret = sprintf(out, "%s: unknown host\n", host);
		write(2,out,pret);
		return -1;
	}
	bcopy(hp->h_addr, &server.sin_addr, hp->h_length);
	server.sin_port = htons(P);
	
	if (connect(sock,(struct sockaddr *) &server,sizeof(server)) < 0) 
	{
		//TODO: connection refused specific error handle
		perror("connecting stream socket");
		return -1;
	}
	return sock;
}

//tokenization
//TODO: realloc if size greater than bufferer
char **token(char *line)
{
	char **input = malloc(sizeof(char*)*BUFFSIZE);
	char *token = malloc(sizeof(char*)*BUFFSIZE);
	char *reserve;
	int position=0;
	token = strtok_r(line," ",&reserve);
	while(token!=NULL)
	{
		input[position]=token;
		position++;
		token = strtok_r (NULL," ",&reserve);
	}
	input[position]='\0';
	return input;
}
