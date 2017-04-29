/*
----------------------------------------------------------------
             System Programming Project 
                  Second Milestone
----------------------------------------------------------------
            Muhammad Taha Shaikh ERP#09415                      
*/
//TODO: fix formatting
//TODO: pipe par data baad mei pohanch raha hai
//TODO: kill error
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <libgen.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <fcntl.h>   
#include <time.h>
#include "processtable.h"


//signal handler for sigchld
void sigcld_handler(int signalnumber){
  int pid;
  int status;
  pid = waitpid(-1, &status, WNOHANG);
  while(pid!= -1)
  {
    if (pid==0)
    {
      return;
    }
    else{
     struct timeval end;
     int gettimeret = gettimeofday(&end,NULL);
     if(gettimeret < 0){
      perror("error in gettimeofday");
      exit(-1);
    }
    insertendtime(pid,end);
    char *tmpout = malloc(sizeof(char*)*MEDIUMBUFFER);
    int sscanret = sscanf(tmpout,"%d was terminated\n",&pid);
    write(1,tmpout,sscanret);
    pid = waitpid(-1, &status, WNOHANG);
  }
}
return;
}

int main()
{
    if(pipe(mainpipe)<0)
  {
    perror("error in pipe");
    exit(-1);
  }

  if(pipe(secondpipe)<0)
  {
    perror("error in pipe");
    exit(-1);
  }

  signal(SIGCHLD,sigcld_handler);

  int childpid = fork();

  if(childpid==0)
  {

    close(PARENT_READ_FD); //closing read end of mainpipe
    close(PARENT_WRITE_FD); //closing write end of the secondpipe
    char * mainpipe_buff = malloc(sizeof(char*)*SMALLBUFFER);
    char * secondpipe_buff = malloc(sizeof(char*)*SMALLBUFFER);

    sprintf(mainpipe_buff,"%d",CHILD_WRITE_FD); //sending write end of main pipe
    sprintf(secondpipe_buff,"%d",CHILD_READ_FD); //sending read end of second pipe*/

    int execret;

    execret = execl("/usr/bin/xterm","xterm","-e","./client",mainpipe_buff,secondpipe_buff,NULL);

    if(execret==-1)
    {

      perror("error in execl");
      exit(-1);

    }
  }
  else if(childpid>0)
  {

    close(CHILD_READ_FD); //closing write end of the mainpipe
    close(CHILD_WRITE_FD); //closing read end of the second pipe

    char *buffer = malloc(sizeof(char*)*BUFFSIZE);
    char **input;

    while(1)
    {
      int readct = read(PARENT_READ_FD,buffer,BUFFSIZE);

      if (readct<0)
      {
        perror("error reading from pipe");
        close(PARENT_READ_FD); 
        close(PARENT_WRITE_FD); 
        free(buffer);
        exit(-1);
      }
      else
      {
        input = token(buffer);
        char *tokenfirst = input[0];

        sscanf(*input,"%s",tokenfirst);
        if(strcasecmp(tokenfirst,"exit")==0)
        {
          free(buffer);
          free(input);
          kill(childpid,SIGTERM);
          exit(0);
        }
        else if (strcasecmp(tokenfirst,"run")==0)
        {
          int pid = run(input[1]);
          struct timeval start;
          int gettimeret = gettimeofday(&start,NULL);

          if(gettimeret < 0)
          {
            perror("error in gettimeofday");
            exit(-1);
          }

          if(pid!=0)
          {
            char *name = basename(input[1]); //get filename from path
            char *namecpy = malloc(strlen(name));

            strcpy(namecpy,name);
            insertfirst(pid,namecpy,start);
            write(PARENT_WRITE_FD,"Success\n",sizeof("Success\n"));
            continue;
          }
          else
          {
            write(PARENT_WRITE_FD,"Bad Name\n",sizeof("Bad Name\n"));
            continue;
          }
        }
        else if(strcasecmp(tokenfirst,"list")==0)
        {
          if(input[1]!=NULL)
          {
            listall(); 
          }
          else
          {
            list();
          }

        }
        else if(strcasecmp(tokenfirst,"kill")==0)
        {
          if(input[1]==NULL)
          {
           write(PARENT_WRITE_FD,"Enter process id or name\n",sizeof("Enter process id or name\n"));
          }
          else
          {
            if(strcasecmp(input[1],"all")==0)
            {
              killallprocess(input[2]);
            }
            else
            {
               killoneprocess(input[1]);
            }
          }
        }
        else if(strcasecmp(tokenfirst,"add")==0)
        {
          *input="0";
          add(input); //add function     
        }
        else if(strcasecmp(tokenfirst,"mul")==0)
        {
          *input="1";
          multiply(input); //multiply function
        }
        else if(strcasecmp(tokenfirst,"sub")==0)
        {
          *input="0";
          subtract(input); //subtract function
        }
       else if(strcasecmp(tokenfirst,"div")==0)
       {
          if(input[3]!=NULL || input[2]==NULL)
          {
            write(PARENT_WRITE_FD,"Divide between 2 numbers please...\n",sizeof("Divide between 2 numbers please...\n"));
          }
          else if(strcasecmp(input[2],"0")==0)
          {
            write(PARENT_WRITE_FD,"Divide by zero not allowed...\n",sizeof("Divide by zero not allowed...\n"));
          }
          else
          {
            float div =0;
            char *end;
            div=(float)(strtof(input[1],&end)/strtof(input[2],&end)); //divide function
            if(end==input[1]||end==input[2])
            {
              write(PARENT_WRITE_FD,"Don't divide strings!\n",sizeof("Don't divide strings!\n"));
            }
            else
            {
              char *tempout=malloc(sizeof(char*)*20);
              sprintf(tempout,"%.2f\n",div);
              write(PARENT_WRITE_FD,tempout,sizeof(tempout));
            }
          }
        }
        else if(strcasecmp(tokenfirst,"help")==0)
        {
          char *help = "mul\tEnter numbers to multiply\n"
          "sub\tEnter numbers to subtract\ndiv\tEnter numbers to divide\nadd\tEnter numbers to add\n"
          "print\tEnter a message to print it\nexit\tTerminate the shell\n";
          write(PARENT_WRITE_FD,help,strlen(help));
        }
        else if(strcasecmp(tokenfirst,"print")==0)
        {
          input[0]="";
          char * tempOutput = malloc(sizeof(char*)*BUFFSIZE);

          for(; *input!='\0'; input++)
          {
            int c = sprintf(tempOutput,"%s",*input);
            write(PARENT_WRITE_FD,tempOutput,c);
          }

          write(PARENT_WRITE_FD,"\n",sizeof("\n"));
        }
        else if(strcasecmp(tokenfirst,"\n")==0)
        {
          continue;
        }
        else
        {
         write(PARENT_WRITE_FD,"Wrong command...type 'help'\n",sizeof("Wrong command...type 'help'\n"));
        }
      }
    }
    free(buffer);
    close(PARENT_WRITE_FD);
    close(PARENT_READ_FD);
    exit(0);
  }
  else
  {
    perror("error in fork");
    exit(-1);
  }
}

  //kill one process
  void killoneprocess(char *process)
  {
    if(process[0] >= 48 && process[0] <= 57)
    {
      int id;
      sscanf(process,"%d",&id);
      if (id <=0 )
      {
        write(PARENT_WRITE_FD,"Enter pid correctly\n",sizeof("Enter pid correctly\n"));
      }
      else
      {
       struct processinfo *prc = searchpid(id);
       if(prc!=NULL && prc->isrunning!=0)
       {
        struct timeval end;
        int gettimeret = gettimeofday(&end,NULL);
        if(gettimeret < 0)
        {
          perror("error in gettimeofday");
          exit(-1);
        }
        int killret = kill(prc->pid,SIGTERM);
        if(killret<0)
        {
          perror("error in kill");
          exit(-1);
        }
        write(PARENT_WRITE_FD,"terminated successfully\n",sizeof("terminated successfully\n"));
      }
      else
      {
        write(PARENT_WRITE_FD,"Process does not exist in the list\n",sizeof("Process does not exist in the list\n"));
      }
    }
  }
  else{
    struct processinfo *prc = searchpname(process,head);
    printf("%s\n", prc->processname);
    if(prc!=NULL)
    {
      if(prc->isrunning!=0)
      {
        int killret = kill(prc->pid,SIGTERM);
        if(killret<0)
        {
          perror("error in kill");
          exit(-1);
        }
        write(PARENT_WRITE_FD,"terminated successfully\n",sizeof("terminated successfully\n"));
      }
      else
      {
        write(PARENT_WRITE_FD,"All instances have already been killed\n",sizeof("All instances have already been killed\n"));
      }
    }
    else
    {
      write(PARENT_WRITE_FD,"Process does not exist in the list\n",sizeof("Process does not exist in the list\n"));
    }
  }
}
//end of kill

//kill all processes with same name function
void killallprocess(char *process)
{
  struct processinfo *prc = searchpname(process,head);
  if(prc!=NULL)
  {
    if(prc->isrunning!=0)
    { 
      while(prc!=NULL)
      { printf("hello\n");
        int killret = kill(prc->pid,SIGTERM);
        if(killret<0)
        {
          perror("error in kill");
          exit(-1);
        }
        prc = searchpname(process,prc->nextprocess);
      }
      write(PARENT_WRITE_FD,"All instances have been killed\n",sizeof("All instances have been killed\n"));
    }
    else
    {
      write(PARENT_WRITE_FD,"All instances have already been killed\n",sizeof("All instances have already been killed\n"));
    }
  }
  else
  {
    write(PARENT_WRITE_FD,"Process does not exist in the list\n",sizeof("Process does not exist in the list\n"));
  }
//end of kill
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

//run process
int run (char args[])
{
  int pfd[2];
  int piperet = pipe2(pfd,O_CLOEXEC);
  if (piperet < 0)
  {
    perror("error in run pipe");
    exit(-1);
  }
  pid_t pid = fork();
  if(pid==0)
  {
    close(pfd[0]);
    sscanf(args,"%s",args);
    execlp(args,args,NULL);
    perror("execl");
    return 0;
  }
  else if(pid>0)
  {
    close(pfd[1]);
    int readct;
    char * buff = malloc(sizeof(char*)*MEDIUMBUFFER);
    readct = read(pfd[0],buff,MEDIUMBUFFER);
    if (readct<0)
    {
      write(PARENT_WRITE_FD,"error during exec\n",sizeof("error during exec\n"));
      return 0;
    }
    else
    {
      return pid;
    }
  }
  else
  {
    perror("fork");
    exit(-1);
  }

}

//subtract function
void subtract(char **nums)
{
  long val = 0;
  char *end=NULL;
  char *out=malloc(sizeof(char*)*20);
  errno = 0;
  for(;*nums!='\0'; nums++)
  {
    long temp = strtol(*nums, &end, 10);
    if(end==*nums)
    {
      write(PARENT_WRITE_FD,"Don't subtract strings!\n",sizeof("Don't subtract strings!\n"));
      return;
    }
    val=abs(val)-abs(temp);
  }

  sprintf(out,"%ld\n",val);
  write(PARENT_WRITE_FD,out,sizeof(out));

}
//multiply function
void multiply(char **nums)
{
  long val = 1;
  char *end;
  char *out=malloc(sizeof(char*)*20);
  for(;*nums!='\0'; nums++)
  {
    val*= strtol(*nums, &end, 10);
    if(end==*nums)
    {
      write(PARENT_WRITE_FD,"Don't multiply strings!\n",sizeof("Don't multiply strings!\n"));
      return;
    }
  }
  sprintf(out,"%ld\n",val);
  write(PARENT_WRITE_FD,out,sizeof(out));
}

//add function
void add(char **nums)
{
  long val = 0;
  char *end;
  char *out=malloc(sizeof(char*)*20);
  for(;*nums!='\0'; nums++)
  {
    val+= strtol(*nums, &end, 10);
    if(end==*nums)
    {
      write(PARENT_WRITE_FD,"Don't add strings!\n",sizeof("Don't add strings!\n"));
      return;
    }
  }
  sprintf(out,"%ld\n",val);
  write(PARENT_WRITE_FD,out,20);
}


/*---------------------------------------------------------------
                      process list functions
---------------------------------------------------------------*/
//display the list
void listall() 
{
  struct processinfo *ptr = head;
  char header[]="PID\tName\tStartTime\tEndTime\tIsRunning\n";
  write(PARENT_WRITE_FD,header,sizeof(header));
  char *outputstring = malloc(sizeof(char*)*BUFFSIZE);
   //start from the beginning
  while(ptr != NULL) 
  {
    int pid = ptr->pid;
    char *name = ptr->processname;
    char *starttime = printabletime(ptr->starttime);
    char *endtime = printabletime(ptr->endtime);
    int isrunning=ptr->isrunning;
    sprintf(outputstring,"%d\t%s\t%s\t%s\t%d\n",pid,name,starttime,endtime,isrunning);
    write(PARENT_WRITE_FD,outputstring,strlen(outputstring));
    ptr = ptr->nextprocess;
  }
}

char *printabletime(struct timeval tv)
{
  char*tmbuf =  malloc(sizeof(char*)*64);
  time_t nowtime;
  struct tm *nowtm;
  nowtime = tv.tv_sec;
  nowtm = localtime(&nowtime);
  strftime(tmbuf, sizeof(tmbuf), "%M:%S", nowtm);
  return tmbuf;
}

//list running children
void list()
{
  struct processinfo *ptr = head;
  if (ptr == NULL)
  {
    write(PARENT_WRITE_FD,"No processes in the list\n",sizeof("No processes in the list\n"));
    return;
  }
  printf("%s\n",(head->processname));
  char header[]="PID\tName\t\tStartTime\t\tIsRunning\n";
  write(PARENT_WRITE_FD,header,sizeof(header));
  char *outputstring = malloc(sizeof(char*) * BUFFSIZE);
  while(ptr->isrunning!=0 && ptr != NULL) 
  {
    int pid = ptr->pid; 
    char *starttime = printabletime(ptr->starttime);
    int isrunning=ptr->isrunning;
    int printret = sprintf(outputstring,"%d\t%s\t\t%s\t\t%d\n",pid,ptr->processname,starttime,isrunning);
    write(PARENT_WRITE_FD,outputstring, printret);
    if(ptr->nextprocess == NULL)
    {
      return;
    }
    else
    {
      ptr = ptr->nextprocess;
    }
  }
}

//insert end time
void insertendtime(int pid, struct timeval end) 
{
  struct processinfo *link = searchpid(pid);
  link->endtime = end;
  link->isrunning = 0;
}

//insert link at the first location
void insertfirst(int pid, char *name, struct timeval start) 
{
   //create a link
  struct processinfo *link = (struct processinfo*) malloc(sizeof(struct processinfo));
  link->processname = malloc(sizeof(char*)*MEDIUMBUFFER);
  link->pid = pid;
  link->processname = name;
  link->isrunning = 1;
  link->starttime = start;
  link->nextprocess = head;
   //point first to new first node
  head = link;
}

struct processinfo *searchpid(int pid) 
{
   //start from the first link
  struct processinfo *current = head;

   //if list is empty
  if(head == NULL) 
  {
    return NULL;
  }
   //navigate through list
  while(current->pid != pid) 
  {

      //if it is last node
    if(current->nextprocess == NULL) 
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

struct processinfo *searchpname (char *name,struct processinfo *ptr) 
{
  struct processinfo *current = ptr;
  if(ptr==NULL)
  {
    return NULL;
  }
   //navigate through list
  while(strcasecmp(current->processname,name)!=0) 
  {

      //if it is last node
    if(current->nextprocess == NULL) 
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

