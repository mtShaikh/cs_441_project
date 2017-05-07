#ifndef PROCESSLIST_H_INCLUDED
#define PROCESSLIST_H_INCLUDED

#include <time.h>
#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MEDIUMBUFFER 64

typedef struct process
{
  int pid;
  char *processname;
  struct timeval starttime;
  struct timeval endtime;
  int isrunning;
  struct process *nextprocess;
} processinfo;

processinfo *head = NULL;

char *
printabletime (struct timeval tv)
{
  char *tmbuf = malloc(sizeof(char*));
  time_t nowtime;
  struct tm *nowtm;
  nowtime = tv.tv_sec;
  nowtm = localtime (&nowtime);
  strftime (tmbuf, sizeof (tmbuf), "%M:%S", nowtm);
  return tmbuf;
}

processinfo *
searchpid (int pid)
{
  //start from the first link
  processinfo *current = head;

  //if list is empty
  if (head == NULL)
    {
      return NULL;
    }
  //navigate through list
  while (current->pid != pid)
    {

      //if it is last node
      if (current->nextprocess == NULL)
        {
          return NULL;
        }
      else
        {
          //go to next link
          current = current->nextprocess;
        }
    }
  //if data found, return the current Link
  return current;
}

processinfo *
searchpname (char *name, processinfo *ptr)
{
  processinfo *current = ptr;
  if (ptr == NULL)
    {
      return NULL;
    }
  //navigate through list
  while (strcasecmp (current->processname, name) != 0)
    {

      //if it is last node
      if (current->nextprocess == NULL)
        {
          return NULL;
        }
      else
        {
          //go to next link
          current = current->nextprocess;
        }
    }
  //if data found, return the current Link
  return current;
}

//insert end time
void
insertendtime (int pid, struct timeval end)
{
  processinfo *link = searchpid (pid);
  link->endtime = end;
  link->isrunning = 0;
}

//insert link at the first location
void
insertfirst (int pid, char *name, struct timeval start)
{
  //create a link
  processinfo *link =
    (struct process *) malloc (sizeof (struct process));
  link->processname = malloc (sizeof (char *) * MEDIUMBUFFER);
  link->pid = pid;
  link->processname = name;
  link->isrunning = 1;
  link->starttime = start;
  link->nextprocess = head;
  //point first to new first node
  head = link;
}


#endif // PROCESSLIST_H_INCLUDED