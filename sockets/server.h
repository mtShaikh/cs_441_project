#ifndef SERVER_H_INCLUDED
#define SERVER_H_INCLUDED
#include <time.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include "processlist.h"
#include <signal.h>

#define BUFFSIZE 1024
#define SMALLBUFFER 10
#define MEDIUMBUFFER 64

int total_clients;
pthread_mutex_t mutex;

#define ERROR(m) \
    do \
    { \
        perror(m); \
        exit(EXIT_FAILURE); \
    } while(0)

typedef void* (*tfunction)(void*);

void grimreaper (int signalnumber);
void reap_subservers(int signo);
void print_client(int signo, siginfo_t *info,void *ctx);
void terminatechildren (int signo);

char **token(char *line);
int run (char args[], int sockfd);
void *subtract(void *args);
void *multiply(void *args);
void *add(void *args);
void *divide(void *args);
void killallprocess(char *process, int sockfd);
void killoneprocess(char *process, int sockfd);
void killall();
int requesthandler(int fdsock);
void listall(int sockfd);
void list(int sockfd);
int initServer();
void *readThread(void *args);
void disconnect(char *client);

//sending data to threads
typedef struct data
{
	int sockfd;
	char *args;
} argument_list;

typedef struct connection {
 int clientno;
 int fdsock;
 int pid;
 char *clientaddress;
 int port;
 int status;
 struct connection *nextclient;
} client;

client *firstclient = NULL;

void insertclient (int pid,int fdsock, int clientno, char *clientaddress, int port, int status);
client *searchclientaddress (char *ip);
client *searchclient (int clientno);
client *searchclientpid (int pid);
char *changeclientstatus(int pid);

#endif // SERVER_H_INCLUDED
