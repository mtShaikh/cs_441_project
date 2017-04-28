/*
----------------------------------------------------------------
             System Programming Project 
                  First Milestone
----------------------------------------------------------------
            Muhammad Taha Shaikh ERP#09415                      
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <libgen.h>
#define BUFFSIZE 5000
#include "functions.h"

int main(int argc, char*argv[])
{
  char *buffer;
  char **input;
  buffer = malloc(sizeof(char*)*BUFFSIZE);
  while(1){
    write(STDOUT_FILENO,">>",sizeof(">>"));
    int count = read(STDIN_FILENO,buffer,BUFFSIZE);
    if(count<0){
      perror("Error:");
      exit(1);
    }
    else{
      input = token(buffer);
      char *tokenFirst= *input;
      sscanf(*input,"%s",tokenFirst);
      if(strcasecmp(tokenFirst,"exit")==0)
      {
        free(buffer);
        free(input);
        exit(0);
      }
      else if (strcasecmp(tokenFirst,"run")==0){
        run(input[1]);
        continue;
      }
      else if(strcasecmp(tokenFirst,"add")==0)
      {
        *input="0";
            add(input); //add function
          }
          else if(strcasecmp(tokenFirst,"mul")==0)
          {
            *input="1";
            multiply(input); //multiply function
          }
          else if(strcasecmp(tokenFirst,"sub")==0)
          {
            *input="0";
            subtract(input); //subtract function
          }
          else if(strcasecmp(tokenFirst,"div")==0)
          {
            if(input[3]!=NULL || input[2]==NULL)
            {
              write(1,"Divide between 2 numbers please...\n",sizeof("Divide between 2 numbers please...\n"));
            }
            else if(strcasecmp(input[2],"0")==0)
            {
              write(1,"Divide by zero not allowed...\n",sizeof("Divide by zero not allowed...\n"));
            }
            else
            {
              float div =0;
              char *end;
                div=(float)(strtof(input[1],&end)/strtof(input[2],&end)); //divide function
                if(end==input[1]||end==input[2]){
                  write(1,"Don't subtract strings!\n",sizeof("Don't subtract strings!\n"));
                }
                else
                {
                  char *tempout=malloc(sizeof(char*)*20);
                  sprintf(tempout,"%.2f\n",div);
                  write(1,tempout,sizeof(tempout));
                }
              }
            }
            else if(strcasecmp(tokenFirst,"help")==0)
            {
              write(STDOUT_FILENO,"mul   Enter numbers to multiply\n",sizeof("mul   Enter numbers to multiply\n"));
              write(STDOUT_FILENO,"sub   Enter numbers to subtract\n",sizeof("sub   Enter numbers to subtract\n"));
              write(STDOUT_FILENO,"div   Enter numbers to divide\n",sizeof("div   Enter numbers to divide\n"));
              write(STDOUT_FILENO,"add   Enter numbers to add\n",sizeof("add   Enter numbers to add\n"));
              write(STDOUT_FILENO,"print Enter a message to print it\n",sizeof("print Enter a message to print it\n"));
              write(STDOUT_FILENO,"exit  Terminate the shell\n",sizeof("exit  Terminate the shell\n"));
            }
            else if(strcasecmp(tokenFirst,"print")==0)
            {
              input[0]="";
              char * tempOutput = malloc(sizeof(char*)*BUFFSIZE);
              for(; *input!='\0'; input++)
              {
                int c = sprintf(tempOutput,"%s",*input);
                write(STDOUT_FILENO,tempOutput,c);
              }
              write(STDOUT_FILENO,"\n",sizeof("\n"));
            }
            else if(strcasecmp(tokenFirst,"\n")==0){
              continue;
            }
            else
            {
              write(STDOUT_FILENO,"Wrong command...type 'help'\n",sizeof("Wrong command...type 'help'\n"));
            }
          }
        }
      }

      //tokenization
char **token(char *line){
    char **input = malloc(sizeof(char*)*BUFFSIZE);
    char *token = malloc(sizeof(char*)*BUFFSIZE);
    int position=0;
    token= strtok(line," ");
    while(token!=NULL){
      input[position]=token;
      position++;
      token = strtok (NULL," ");
  }
  input[position]='\0';
  return input;
}

//run process
void run (char args[]){
    pid_t pid = fork();
    if(pid==0){
         int exec_ret;
        char *name_of_file = basename(args);
        sscanf(args,"%s",args);
        exec_ret = execlp(name_of_file,args,NULL);
        if(exec_ret== -1){
            perror("execl");
            exit(0);
        }
    }
    else if(pid>0){
        return;
    }
    else{
        perror("fork");
        return;
    }

}


//subtract function
void subtract(char **nums){
    long val = 0;
    char *end=NULL;
    char *out=malloc(sizeof(char*)*20);
    errno = 0;
    for(;*nums!='\0'; nums++)
    {
        long temp = strtol(*nums, &end, 10);
        if(end==*nums){
            write(1,"Don't subtract strings!\n",sizeof("Don't subtract strings!\n"));
            return;
        }
        val=abs(val)-abs(temp);
    }
    
        sprintf(out,"%ld\n",val);
        write(1,out,sizeof(out));
    
}
//multiply function
void multiply(char **nums){
    long val = 1;
    char *end;
    char *out=malloc(sizeof(char*)*20);
    for(;*nums!='\0'; nums++)
    {
        val*= strtol(*nums, &end, 10);
        if(end==*nums){
            write(1,"Don't subtract strings!\n",sizeof("Don't subtract strings!\n"));
            return;
        }
    }
    sprintf(out,"%ld\n",val);
    write(1,out,sizeof(out));
}

//add function
void add(char **nums){
    long val = 0;
    char *end;
    char *out=malloc(sizeof(char*)*20);
    for(;*nums!='\0'; nums++)
    {
        val+= strtol(*nums, &end, 10);
        if(end==*nums){
            write(1,"Don't subtract strings!\n",sizeof("Don't subtract strings!\n"));
            return;
        }
    }
    sprintf(out,"%ld\n",val);
    write(1,out,sizeof(out));
}

