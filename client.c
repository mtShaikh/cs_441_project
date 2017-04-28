#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#define BUFFSIZE 5000

int main(int argc, char*argv[])
{
  int writefd;
  int readfd;
  char* writebuff = malloc(sizeof(char*)*BUFFSIZE);
  char* readbuff = malloc(sizeof(char*)*BUFFSIZE);
  sscanf(argv[1],"%d",&writefd);
  sscanf(argv[2],"%d",&readfd);
  while(1){
    write(1,">>",2);
    int readct = read(0,readbuff,BUFFSIZE);
    if (readct<0)
    {
      perror("error in read");
      exit(-1);
    }
    else if(readct == 1){
      continue;
    }
    int pipewritect = write(writefd,readbuff,readct);
    if (pipewritect<0)
    {
      perror("error in write");
      exit(-1);
    }
    int pipereadct = read(readfd,writebuff,BUFFSIZE);
    if (pipereadct<0)
    {
      perror("error in read");
      exit(-1);
    }
    else if (pipereadct == 0)
    {
      continue;
    }
    int writect = write(1,writebuff,pipereadct);
    if (writect<0)
    {
      perror("error in write");
      exit(-1);
    }
  }

}
  