#ifndef SERVER_H_INCLUDED
#define SERVER_H_INCLUDED
#include <time.h>
#include <sys/time.h>


#define BUFFSIZE 5000
#define SMALLBUFFER 10
#define MEDIUMBUFFER 64
#define READ_FD  0
#define WRITE_FD 1

char **token(char *line);
int run (char args[]);
void subtract(char **nums);
void multiply(char **nums);
void add(char **nums);
void killallprocess(char *process);
void killoneprocess(char *process);
void requesthandler(int fdsock);

struct processinfo {
 int pid;
 char *processname;
 struct timeval starttime;
 struct timeval endtime;
 int isrunning;
 struct processinfo *nextprocess;
};

struct processinfo *head = NULL;
struct processinfo *current = NULL;

void listall();
void list();
void insertfirst(int pid, char *name, struct timeval start);
void insertendtime(int pid, struct timeval end);
char *printabletime(struct timeval tv);
struct processinfo *searchpid(int pid);
struct processinfo *searchpname (char *name,struct processinfo *ptr);

#endif // PROCESSTABLE_H_INCLUDED
