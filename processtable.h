#ifndef PROCESSTABLE_H_INCLUDED
#define PROCESSTABLE_H_INCLUDED
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

int mainpipe[2]; //client writes and server reads
int secondpipe[2]; //server writes and client reads

#define PARENT_READ_FD  ( mainpipe[READ_FD]   )//main[0]
#define PARENT_WRITE_FD ( secondpipe[WRITE_FD] )//second[1]

#define CHILD_READ_FD   ( secondpipe[READ_FD]  )//second[0]
#define CHILD_WRITE_FD  ( mainpipe[WRITE_FD]  )//main[1]

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
