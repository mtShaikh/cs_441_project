/*
----------------------------------------------------------------
             System Programming Project
----------------------------------------------------------------
            Muhammad Taha Shaikh ERP#09415
*/
//TODO: fix formatting
//TODO: free everything
//TODO: error handling for add and other computational operations
//TODO: struct for all subservers
//TODO: threads for all functions-->worker threads
//TODO: read write-->server commands
//set name of client send pid to client
#define _GNU_SOURCE
#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <libgen.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <fcntl.h>
#include <time.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <regex.h>

#include "server.h"
#include "processlist.h"

int
main ()
{
  int sockfd;  
  int pid;
  pthread_t threadid;
  total_clients = 0;
  char clientname[INET_ADDRSTRLEN];
  char *addressbuffer = malloc (sizeof (char *) * MEDIUMBUFFER);    //for printing address
  struct sockaddr_in clientaddress;
  int fdsock = initServer();
  threadid = pthread_create(&threadid, NULL, readThread, NULL);
  if (threadid < 0)
  {
    ERROR("pthread_create");
  }
  int detachret = pthread_detach(threadid); 
  if (detachret < 0)
  {
    ERROR("pthread_detach");
  }
  do
  {
    int clientlen = sizeof(clientaddress);
    sockfd = accept (fdsock,(struct sockaddr *)&clientaddress, &clientlen);
    if (sockfd < 0)
    {
      ERROR ("unable to accept");
    }
    else
    {   
      if(inet_ntop(AF_INET,&clientaddress.sin_addr.s_addr,clientname,sizeof(clientname))!=NULL)
      {
        int pcount =
        sprintf (addressbuffer, "Incoming connection from %s at port %d\n", clientname, clientaddress.sin_port);
        write(1, addressbuffer, pcount);
      }
      pid = fork();
      if(pid < 0)
      {
        ERROR("error while creating child for client");
      }
      else if(pid == 0)
      {
        close(fdsock);
            //passing struct in write typecast to char*
        signal(SIGTERM,terminatechildren);
        signal(SIGUSR1, print_client);
            //in child
        int retval = requesthandler (sockfd);
        if (retval == -1)
        {
          write(1, "Client terminated abruptly\n", sizeof("Client terminated abruptly\n"));
          killall();
          exit(0);
        }
        else if(retval == 0)
        {
          write(1, "Ending connection\n", sizeof("Ending connection\n"));
          close(sockfd);
          killall();
          char *tmpout = changeclientstatus(pid);
          write (1, tmpout, strlen(tmpout));
          exit(0);
                //end all processes of client
        }
      }
      else if (pid > 0)
      {   
        signal(SIGCHLD,reap_subservers);
            //in parent
        insertclient(pid,sockfd, total_clients, clientname, clientaddress.sin_port, 1);
        total_clients++;
      }
    }
  }
  while (1);
  close(fdsock);
}

//initailize server
int initServer()
{
  char *addressbuffer = malloc (sizeof (char *) * SMALLBUFFER);    //for printing address  
  // declaring socket fds
  int fdsock;
  struct sockaddr_in server;
  // creating socket
  fdsock = socket (AF_INET, SOCK_STREAM, 0);
  if (fdsock < 0)
  {
    ERROR("error opening socket");
  }

  // configuring server
  server.sin_family = AF_INET;
  server.sin_port = 0;
  server.sin_addr.s_addr = INADDR_ANY;

  // bind the socket to server
  int b = bind (fdsock, (struct sockaddr *) &server, sizeof (server));
  if (b < 0)
  {
    ERROR("binding socket");
  }

  // find port and address
  int length = sizeof (server);
  if (getsockname
    (fdsock, (struct sockaddr *) &server, (socklen_t *) & length))
  {
    ERROR("getting socket name");
  }
  int pcount =
  sprintf (addressbuffer, "Socket has port #%d\n", ntohs (server.sin_port));
  write (1, addressbuffer, pcount);
  // start accepting connections
  if((listen(fdsock, 5)) == -1)
  {
    ERROR("Server Listen Error");
  }
  return fdsock;
}

//server Read from stdin thread + parser for server commands
void *readThread(void *args)
{
 while(1)
 {      
        write(1,">>",2);
        char *lineBuffer = malloc(sizeof(char*)*BUFFSIZE); // for reading from terminal
        int readcount = read(STDIN_FILENO, lineBuffer, sizeof(lineBuffer));  
        char **line;
        if(readcount < 0)
        {
          ERROR("Server Terminal Read Error");
        }
        line = token(lineBuffer);
        char *tokenfirst = line[0];
        sscanf(*line,"%s",tokenfirst);
        printf("%s\n",tokenfirst );
        if(strcasecmp(tokenfirst,"exit") == 0)
        {
          write(1, "exiting\n", sizeof("exiting\n"));
          free(lineBuffer);
          exit(0);
        }
        else if(strcasecmp(tokenfirst,"dcon") == 0 ||  strcasecmp(tokenfirst,"disconnect") == 0) 
        {
          client *current = firstclient;
          if (current == NULL)
          {
           write(1,"No connections yet\n",sizeof "No connections yet\n");
           continue;
         }
         if(line[0]==NULL)
         {
           char *help = "Error: disconnect [ip4 address] | [client no.]\n";
            write (1, help, strlen (help));
         }
         else
         {
          disconnect(line[1]);
          continue;
        }
      }
        else if ((strcasecmp (tokenfirst, "message") == 0) || (strcasecmp (tokenfirst, "msg") == 0))
        {
              //message subservers
        }
        else if(strcasecmp(tokenfirst,"list") == 0) 
        {
         client *current = firstclient;
         if (current == NULL)
         {
           write(1,"No connections yet\n",sizeof "No connections yet\n");
           continue;
         }
         char*output = malloc(sizeof(char*)*SMALLBUFFER);
         int sprintf_ret;
         while(current!=NULL)
         {
          if(current->status == 1)
          {
            sprintf_ret = sprintf(output,"Client #%d\n has IP Addr %s on Port #%d", current->clientno,current->clientaddress, current->port);
            write(1,output,sprintf_ret);
            kill(current->pid,SIGUSR1);
          }
          else
          {
            sprintf_ret = sprintf(output,"Client #%d having IP Addr %s on Port #%d has been disconnected\n", current->clientno,current->clientaddress, current->port);
            write(1,output,sprintf_ret);
          }
          current = current->nextclient;
        }
      }
      else if(strcasecmp(tokenfirst,"\n")==0)
      {
        continue;
      }
      else
      {
        char *help = "Valid Commands\nlist\tList of connected clients and their processes\n"
      "exit\tTerminate the server\ndisconnect\tDisconnect from specific client\n";
      write (1, help, strlen (help));
      }
    }
    pthread_exit(0);
  }


//disconnect command
  void disconnect(char *clientval)
  {
    char *end;
    int number = strtol (clientval, &end, 10);
    if (end == clientval)
    {
      regex_t regex;
      int regres;
      regres = 
      regcomp(&regex, 
        "^([0-9]|[1-9][0-9]|1([0-9][0-9])|2([0-4][0-9]|5[0-5]))."
        "([0-9]|[1-9][0-9]|1([0-9][0-9])|2([0-4][0-9]|5[0-5]))."
        "([0-9]|[1-9][0-9]|1([0-9][0-9])|2([0-4][0-9]|5[0-5]))."
        "([0-9]|[1-9][0-9]|1([0-9][0-9])|2([0-4][0-9]|5[0-5]))$", REG_EXTENDED);

      if (regres)
      {
        ERROR("Could not compile regex\n");
      }
      regres = regexec(&regex, clientval, 0, NULL, 0);
      if( regres == REG_NOMATCH )
      {
        write(1, "Invalid IP address\n",sizeof "Invalid IP address\n");
        return;
      }
      else
      {
        client *current = searchclientaddress(clientval);
        if (current==NULL)
        {
          write(1,"The address does not exist in the list\n",sizeof "The address does not exist in the list\n");
          return;
        }
        else
        {
          kill(current->pid, SIGTERM);
        }
      }
    }
    else
    {
      client *current = searchclient(number);
      if (current==NULL)
        {
          write(1,"The address does not exist in the list\n",sizeof "The address does not exist in the list\n");
          return;
        }
        else
        {
          kill(current->pid, SIGTERM);
        }
    }
}


//mainloop
int
requesthandler (int fdsock)
{
  pthread_t tid_add;
  pthread_t tid_sub;
  pthread_t tid_mult;
  pthread_t tid_div;
  write (fdsock, "Connected\n", sizeof ("Connected\n"));
  signal (SIGCHLD, grimreaper);
  char *buffer = malloc (sizeof (char *) * BUFFSIZE);
  char **input;
  while (1)
  {
    bzero(buffer,BUFFSIZE);
    errno = 0;
    int readct = read (fdsock, buffer, BUFFSIZE);

    if (readct < 0)
    {
      free (buffer);
      if(errno == ECONNRESET)
      {
        return -1;
      }
    }
    else if (readct == 0)
    {

      free (buffer);
      return 0;
    }
    else
    {
      char *inputline = malloc(strlen(buffer));
      strcpy(inputline,buffer);
      input = token (buffer);
      char *tokenfirst = input[0];
      sscanf (*input, "%s", tokenfirst);
      if (strcasecmp (tokenfirst, "run") == 0)
      {
        int pid = run (input[1], fdsock);
        struct timeval start;
        int gettimeret = gettimeofday (&start, NULL);
        if (gettimeret < 0)
        {
          ERROR ("error in gettimeofday");
        }
        if (pid != 0)
        {
                  char *name = basename (input[1]); //get filename from path
                  char *namecpy = malloc (strlen (name));
                  strcpy (namecpy, name);
                  insertfirst (pid, namecpy, start);
                  write (fdsock, "Success\n", sizeof ("Success\n"));
                  continue;
                }
                else
                {
                  write (fdsock, "Bad Name\n", sizeof ("Bad Name\n"));
                  continue;
                }
              }
              else if (strcasecmp (tokenfirst, "list") == 0)
              {
                if (input[1] != NULL)
                {
                  listall (fdsock);
                }
                else
                {
                  list (fdsock);
                }
              }
              else if (strcasecmp (tokenfirst, "kill") == 0)
              {
                if (input[1] == NULL)
                {
                  write (fdsock, "Enter process id or name\n",  
                   sizeof ("Enter process id or name\n"));
                }
                else
                {
                  if (strcasecmp (input[1], "all") == 0)
                  {
                    killallprocess (input[2], fdsock);
                  }
                  else
                  {
                    killoneprocess (input[1], fdsock);
                  }
                }
              }
              else if (strcasecmp (tokenfirst, "add") == 0)
              {
                argument_list *values_list = (argument_list*)malloc(sizeof(argument_list));
                values_list->sockfd = fdsock;
                values_list->args = inputline;
                tid_add = pthread_create(&tid_add, NULL, add, values_list);
                if (tid_add < 0)
                {
                  ERROR("pthread_create at add");
                }
                if (pthread_detach(tid_add) < 0)
                {
                 ERROR("pthread_create at add");
               }
               continue;
             }
             else if (strcasecmp (tokenfirst, "mul") == 0)
             {
              argument_list *values_list = (argument_list*)malloc(sizeof(argument_list));
              values_list->sockfd = fdsock;
              values_list->args = inputline;

              tid_mult = pthread_create(&tid_mult, NULL, multiply, values_list);
              if (tid_mult < 0)
              {
                ERROR("pthread_create at multiply");
              }
              if (pthread_join(tid_mult,NULL) < 0)
              {
               ERROR("pthread_create at multiply");
             }
             continue;
           }
           else if (strcasecmp (tokenfirst, "sub") == 0)
           {   
            argument_list *values_list = (argument_list*)malloc(sizeof(argument_list));
            values_list->sockfd = fdsock;
            values_list->args = inputline;
            tid_sub = pthread_create(&tid_sub, NULL, subtract, values_list);
            if (tid_sub < 0)
            {
              ERROR("pthread_create at subtract");
            }
            if (pthread_join(tid_sub,NULL) < 0)
            {
             ERROR("pthread_create at subtract");
           }
           continue;
         }
         else if (strcasecmp (tokenfirst, "div") == 0)
         {

           argument_list *values_list = (argument_list*)malloc(sizeof(argument_list));
           values_list->sockfd = fdsock;
           values_list->args = inputline;
           tid_div = pthread_create(&tid_div, NULL, divide, values_list);
           if (tid_div < 0)
           {
            ERROR("pthread_create at divide");
          }
          if (pthread_join(tid_div,NULL) < 0)
          {
           ERROR("pthread_create at divide");
         }
         continue;
       }
       else if (strcasecmp (tokenfirst, "print") == 0)
       {
        input[0] = "";
        char *tempOutput = malloc (sizeof (char *) * BUFFSIZE);
        for (; *input != '\0'; input++)
        {
          int c = sprintf (tempOutput, "%s", *input);
          write (fdsock, tempOutput, c);
        }
              //write (fdsock, "\n", sizeof ("\n"));
      }
      else if (strcasecmp (tokenfirst, "\n") == 0)
      {
        continue;
      }
      else
      {
        write (fdsock, "Wrong command...type 'help'\n",
         sizeof ("Wrong command...type 'help'\n"));
      }
    }

  }
  return 1;
}



/*
=======signal handlers=========
*/
//signal handler for sigusr
void print_client(int signo)
{
  listall(STDOUT_FILENO);
}

//signal handler for sigchld for main server
void reap_subservers(int signo)
{
  int pid;
  int status;
  pid = waitpid (-1, &status, WNOHANG);
  while (pid != -1)
  {
    if (pid == 0)
    {
      return;
    }
    else
    {
      char *tmpout = changeclientstatus(pid);
      write (1, tmpout, strlen(tmpout));
      pid = waitpid (-1, &status, WNOHANG);
    }
  }
  return;
}

//signal handler for sigchld for subserver
void
grimreaper (int signalnumber)
{
  int pid;
  int status;
  pid = waitpid (-1, &status, WNOHANG);
  while (pid != -1)
  {
    if (pid == 0)
    {
      return;
    }
    else
    {
      struct timeval end;
      int gettimeret = gettimeofday (&end, NULL);
      if (gettimeret < 0)
      {
        ERROR("error in gettimeofday");
      }
      insertendtime (pid, end);
      char *tmpout = malloc (sizeof (char *) * MEDIUMBUFFER);
      int sscanret = sscanf (tmpout, "%d was terminated\n", &pid);
      write (1, tmpout, sscanret);
      pid = waitpid (-1, &status, WNOHANG);
    }
  }
  return;
}

//signal handler for SIGTERM to subserver for terminating all the associated threads and processes
void terminatechildren (int signo)
{
  int pid = getpid();
  char *tmpout = changeclientstatus(pid);
  write (1, tmpout, strlen(tmpout));
  killall();
  exit(0);
}


/*
=======kill functions=========
*/

//kill one process
void
killoneprocess (char *process, int sockfd)
{
  if (process[0] >= 48 && process[0] <= 57)
  {
    int id;
    sscanf (process, "%d", &id);
    if (id <= 0)
    {
      write (sockfd, "Enter pid correctly\n",
       sizeof ("Enter pid correctly\n"));
      return;
    }
    else
    {
      processinfo *prc = searchpid (id);
      if (prc != NULL && prc->isrunning != 0)
      {
        int killret = kill (prc->pid, SIGTERM);
        if (killret < 0)
        {
          ERROR("error in kill");
        }
        write (sockfd, "terminated successfully\n",
         sizeof ("terminated successfully\n"));
      }
      else
      {
        write (sockfd, "Process does not exist in the list\n",
         sizeof ("Process does not exist in the list\n"));
      }
    }
  }
  else
  {
    processinfo *prc = searchpname (process, head);
    if (prc != NULL)
    {
      if (prc->isrunning != 0)
      {
        int killret = kill (prc->pid, SIGTERM);
        if (killret < 0)
        {
          ERROR("error in kill");
        }
        write (sockfd, "terminated successfully\n",
         sizeof ("terminated successfully\n"));
      }
      else
      {
        write (sockfd,
         "All instances have already been killed\n",
         sizeof ("All instances have already been killed\n"));
      }
    }
    else
    {
      write (sockfd, "Process does not exist in the list\n",
       sizeof ("Process does not exist in the list\n"));
    }
  }
}

//end of kill

//kill all processes with same name function
void
killallprocess (char *process, int sockfd)
{
  processinfo *prc = searchpname(process, head);
  if (prc != NULL)
  {
    while(prc!=NULL)
    {   
      errno = 0;
      int killret = kill (prc->pid, SIGTERM);
      if(killret < 0)
      {
        if (errno == ESRCH)
        {
          write (sockfd, "Already killed\n",
           sizeof ("Already killed\n"));
        }
        else
        {
          ERROR("kill all");
        }
      }
      
      prc = searchpname(process, prc->nextprocess);
    }
    write (sockfd, "All processes killed\n",
     sizeof ("All processes killed\n"));
  }
  else
  {
    write (sockfd, "Process does not exist in the list\n",
     sizeof ("Process does not exist in the list\n"));
  }
//end of kill
}

// kill all the processes
void killall()
{
  processinfo *current = head;
  if(current == NULL)
  {
    write(1,"No processes spawned\n", sizeof("No processes spawned\n"));
  }
  else
  {

    while(current!=NULL)
    {   
      errno = 0;
      int killret = kill (current->pid, SIGTERM);
      if (killret < 0)
      {
        if (errno == ESRCH)
        {
         continue;
       }
       else
       {
        ERROR("error in killall");
      }
    }

    current = current->nextprocess;
  }
}
}

/*
=======end of kill=========
*/


//tokenization
char **
token (char *line)
{
  char **input = malloc (sizeof (char *) * BUFFSIZE);
  char *token = malloc (sizeof (char *) * BUFFSIZE);
  char *reserve;
  int position = 0;
  token = strtok_r (line, " ", &reserve);
  while (token != NULL)
  {
    input[position] = token;
    position++;
    token = strtok_r (NULL, " ", &reserve);
  }
  input[position] = '\0';
  return input;
}

//run process
int
run (char args[], int sockfd)
{
  int pfd[2];
  int piperet = pipe2 (pfd, O_CLOEXEC);
  if (piperet < 0)
  {
    ERROR("error in run pipe");

  }
  pid_t pid = fork ();
  if (pid == 0)
  {
    close (pfd[0]);
    sscanf (args, "%s", args);
    execlp (args, args, NULL);
    perror ("execl");
    return 0;
  }
  else if (pid > 0)
  {
    close (pfd[1]);
    int readct;
    char *buff = malloc (sizeof (char *) * MEDIUMBUFFER);
    readct = read (pfd[0], buff, MEDIUMBUFFER);
    if (readct < 0)
    {
      write (sockfd, "error during exec\n",
       sizeof ("error during exec\n"));
      return 0;
    }
    else
    {
      return pid;
    }
  }
  else
  {
    ERROR ("fork");
  }

}

//subtract function
void
*subtract (void*args)
{ 
  argument_list *values= (argument_list*) args;
  char **nums = token(values->args);
  int sockfd = values->sockfd;
  int val = 0;
  char *end = NULL;
  char *out = malloc (sizeof (char *) * 20);
  int count = 1;
  errno = 0;
  *nums = "0";
  for (; *nums != '\0'; nums++)
  {
    int temp = strtod (*nums, &end);
    if (end == *nums)
    {
      write (sockfd, "Don't subtract strings!\n",
       sizeof ("Don't subtract strings!\n"));
      pthread_exit(NULL);
    }
    if (count)
    {
     val = temp;
     count = 0;
   }
   else
   {
    val = val - temp;
  }
}
sprintf (out, "%d\n", val);
write (sockfd, out, sizeof (out));
pthread_exit(NULL);
}

//multiply function
void
*multiply (void *args)
{
  argument_list *values= (argument_list*) args;
  char **nums = token(values->args);
  int sockfd = values->sockfd;
  long val = 1;
  char *end;
  char *out = malloc (sizeof (char *) * 20);
  *nums = "1";
  for (; *nums != '\0'; nums++)
  {
    val *= strtol (*nums, &end, 10);
    if (end == *nums)
    {
      write (sockfd, "Don't multiply strings!\n",
       sizeof ("Don't multiply strings!\n"));
      pthread_exit(NULL);
    }
  }
  sprintf (out, "%ld\n", val);
  write (sockfd, out, sizeof (out));
  pthread_exit(NULL);
}

//add function
void
*add (void *args)
{
  argument_list *values= (argument_list*) args;
  char **nums = token(values->args);
  int sockfd = values->sockfd;
  long val = 0;
  char *end;
  char *out = malloc (sizeof (char *) * 20);
  *nums = "0";
  for (; *nums != '\0'; nums++)
  {
    val += strtol (*nums, &end, 10);
    if (end == *nums)
    {
      write (sockfd, "Don't add strings!\n",
       sizeof ("Don't add strings!\n"));
      return NULL;
    }
  }
  sprintf (out, "%ld\n", val);
  write (sockfd, out, 20);;
}
//divide functions 
void *divide(void *args)
{
  argument_list *values= (argument_list*) args;
  char **nums = token(values->args);
  int sockfd = values->sockfd;
  if (nums[3] != NULL || nums[2] == NULL)
  {
    write (sockfd, "Divide between 2 numbers please...\n",
     sizeof ("Divide between 2 numbers please...\n"));
    pthread_exit(NULL);
  }
  else if (strcasecmp (nums[2], "0") == 0)
  {
    write (sockfd, "Divide by zero not allowed...\n",
     sizeof ("Divide by zero not allowed...\n"));
    pthread_exit(NULL);
  }
  else
  {
    char *first_num = nums[1];
    char *second_num = nums[2];

    float div = 0;
    char *end;
                  div = (float) (strtof (first_num, &end) / strtof (second_num, &end));    //divide function
                  if (end == first_num || end == second_num)
                  {
                    write (sockfd, "Don't divide strings!\n",
                     sizeof ("Don't divide strings!\n"));
                    pthread_exit(NULL);
                  }
                  else
                  {
                    char *tempout = malloc (sizeof (char *) * 20);
                    sprintf (tempout, "%.2f\n", div);
                    write (sockfd, tempout, sizeof (tempout));
                  }
                }
                pthread_exit(NULL);
              }

/*---------------------------------------------------------------
                     list functions
---------------------------------------------------------------*/

//send whole struct to the client
//display the list
              void
              listall (int sockfd)
              {
                processinfo *ptr = head;
                char header[] = "PID\tName\t\tStartTime\t\tEndTime\tIsRunning\n";
                write (sockfd, header, sizeof (header));
                char *outputstring = malloc (sizeof (char *) * BUFFSIZE);
  //start from the beginning
                while (ptr != NULL)
                {
                  int pid = ptr->pid;
                  char *name = ptr->processname;
                  char *starttime = printabletime (ptr->starttime);
                  char *endtime = printabletime (ptr->endtime);
                  int isrunning = ptr->isrunning;
                  sprintf (outputstring, "%d\t%s  %s  %s\t%d\n", pid, name, starttime,
                   endtime, isrunning);
                  write (sockfd, outputstring, strlen (outputstring));
                  ptr = ptr->nextprocess;
                }
              }

//list running children
              void
              list (int sockfd)
              {
                processinfo *ptr = head;
                if (ptr == NULL)
                {
                  write (sockfd, "No processes in the list\n",
                   sizeof ("No processes in the list\n"));
                  return;
                }

                char header[] = "PID\tName\t\tStartTime\t\tIsRunning\n";
                write (sockfd, header, sizeof (header));
                char *outputstring = malloc (sizeof (char *) * BUFFSIZE);
                while (ptr->isrunning != 0 && ptr != NULL)
                {
                  int pid = ptr->pid;
                  char *starttime = printabletime (ptr->starttime);
                  int isrunning = ptr->isrunning;
                  int printret =
                  sprintf (outputstring, "%d\t%s\t\t%s\t\t%d\n", pid, ptr->processname,
                   starttime, isrunning);
                  write (sockfd, outputstring, printret);
                  if (ptr->nextprocess == NULL)
                  {
                    return;
                  }
                  else
                  {
                    ptr = ptr->nextprocess;
                  }
                }
              }

/*---------------------------------------------------------------
                    end of list functions
---------------------------------------------------------------*/


/*---------------------------------------------------------------
                    Client list functions
---------------------------------------------------------------*/
//insert link at the first location
              void
              insertclient (int pid,int fdsock, int clientno,  char *clientaddress, int port, int status)
              {
  //create a link
                client *link =
                (client *) malloc (sizeof (client));
                link->clientaddress = clientaddress;
                link->port = port;
                link->fdsock = fdsock;
                link->clientno = clientno;
                link->pid = pid;
                link->status = status;
                link->nextclient = firstclient;
  //point first to new first node
                firstclient = link;
              }


              client *
              searchclient (int clientno)
              {
  //start from the first link
                client *current = firstclient;

  //if list is empty
                if (firstclient == NULL)
                {
                  return NULL;
                }
  //navigate through list
                while (current->clientno!= clientno)
                {

      //if it is last node
                  if (current->nextclient == NULL)
                  {
                    return NULL;
                  }
                  else
                  {
          //go to next link
                    current = current->nextclient;
                  }
                }
  //if data found, return the current Link
                return current;
              }


              client *
              searchclientaddress (char *ip)
              {
  //start from the first link
                client *current = firstclient;
  //if list is empty
                if (firstclient == NULL)
                {
                  return NULL;
                }
  //navigate through list
                while (strcmp(current->clientaddress, ip)!=0)
                {

      //if it is last node
                  if (current->nextclient == NULL)
                  {
                    return NULL;
                  }
                  else
                  {
          //go to next link
                    current = current->nextclient;
                  }
                }
                
  //if data found, return the current Link
                return current;
              }

              client *
              searchclientpid (int pid)
              {
  //start from the first link
                client *current = firstclient;

  //if list is empty
                if (firstclient == NULL)
                {
                  return NULL;
                }
  //navigate through list
                while (current->pid!= pid)
                {

      //if it is last node
                  if (current->nextclient == NULL)
                  {
                    return NULL;
                  }
                  else
                  {
          //go to next link
                    current = current->nextclient;
                  }
                }
  //if data found, return the current Link
                return current;
              }


              char *changeclientstatus(int pid)
              {
                client *target = searchclientpid(pid);
                target->status = 0;
                close(target->fdsock);
                char *tmpout = malloc (sizeof (char *) * MEDIUMBUFFER);
                sscanf (tmpout, "%d was terminated with ip address %s at port %d\n", &(target->clientno),target->clientaddress, &(target->port));
                return tmpout;
              }