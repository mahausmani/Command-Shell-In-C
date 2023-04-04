//source for pipedexecution: https://stackoverflow.com/questions/8082932/connecting-n-commands-with-pipes-in-a-shell
//source for queues https://www.digitalocean.com/community/tutorials/queue-in-c

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <assert.h>
#include <sys/wait.h>
#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "parse.c"
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


int infile;
int outfile;
int status;
int counter = 0; //bg jobs
char* history[10] = {NULL};//queue
int Rear = - 1;
int Front = - 1;
int SIZE = 10;
void enqueue(char* insert_item)
{
     
    if (Rear == SIZE - 1)
       printf("Overflow \n");
    else
    {
        if (Front == - 1)
      
        Front = 0;
        
        Rear = Rear + 1;
        history[Rear] = insert_item;
    }
} 
 
void dequeue()
{
    if (Front == - 1 || Front > Rear)
    {
        printf("Underflow \n");
        return ;
    }
    else
    {
        // printf("Element deleted from the Queue: %d\n", inp_arr[Front]);
        Front = Front + 1;
    }
} 
 
void show()
{
    
    if (Front == - 1)
        printf("Empty Queue \n");
    else
    {
        printf("History: \n");
        for (int i = Front; i <= Rear; i++)
            

                printf("%d: %s \n",i, history[i]);
            
        printf("\n");
    }
} 
typedef struct bg_job {
    int pid;
    char* cmdname;
}bg_job;
bg_job jobs[10];
void handler(int s){
    printf("in handler");
}
int spawn_proc (int infile, int outfile, struct commandType cmd)
{
  int pid;
  if ((pid = fork ()) == 0)
    {
      if (infile != 0)
        {
          dup2 (infile, 0);
          close (infile);
        }

      if (outfile != 1)
        {
          dup2 (outfile, 1);
          close (outfile);
        }

      return execvp (cmd.VarList[0],cmd.VarList);
    }

  return pid;
}
void help();
void cd(char *path);
void killall();
void ncmd();
void exit();
void showhistory();
int hist = 0;

int fork_pipes (int n, struct commandType cmd[])
{
  int i;
  int pid;
  int in, fd [2];
  in = 0;
  for (i = 0; i <= n - 1; ++i)
    {
      pipe (fd);
      spawn_proc (in, fd [1], cmd[i]);
      close (fd [1]);
      in = fd [0];
    }
  if (in != 0)
    dup2 (in, 0);
    
  return execvp (cmd[i].VarList [0], cmd[i].VarList);
}
void help(){
    printf("1. jobs - provides a list of all background processes and their local pIDs.\n");
    printf("2. cd PATHNAME - sets the PATHNAME as working directory.\n");
    printf("3. history - prints a list of previously executed commands. Assume 10 as the maximum history\n");
    printf("4. kill PID - terminates the background process identified locally with PID in the jobs list.\n");
    printf("5. !CMD - runs the command numbered CMD in the command history.\n");
    printf("6. exit - terminates the shell only if there are no background jobs.\n");
    printf("7. help - prints the list of builtin commands along with their description.\n");
}
void cd(char *path){
  
    chdir(path);
}
int main(int argc, char *argv[]) 
{   
    while(1){

        const char *prompt = "maha $ ";
        char* cmdline = readline(prompt);
        parseInfo *p = parse(cmdline); 
        int argNum = p->CommArray[0].VarNum;
        char *myargs[MAX_VAR_NUM] = {NULL};
        myargs[0] = p->CommArray[0].command;   
        for (int i = 0;  i <=argNum;i++){
                    myargs[i] = p->CommArray[i].VarList[i-1];
                }
        if (strcmp(p->CommArray[0].command,"exit")==0 ){return 0;}
        enqueue(cmdline);
        hist++;
        if (hist == 10){
            dequeue();
            enqueue(cmdline);
            }
        int rc = fork();
        
        if (rc < 0) {
            fprintf(stderr, "fork failed\n");
            exit(1);
        
        }else if (rc == 0) {
            if (strcmp(p->CommArray[0].command,"exit")==0 ){
                exit(1);
                return 0;}
            if (strcmp(myargs[0], "!CMD") == 0)
				{
					int i = atoi(myargs[1]);
					if (i > 10)
						printf("Invalid index\n");
					else if (history[i] == NULL)
						printf("empty\n");
					else
					{
						
						{
							p = parse(history[i]);
							for (int i = 0; i <= 10; i++)
								myargs[i] = p->CommArray[0].VarList[i];
						}
					}
                    execvp(myargs[0], myargs);
				}
            // print_info(p);
            if (strcmp(myargs[0],"help")==0 ){help();}
            if (strcmp(myargs[0],"cd")==0 ){
                cd(p->CommArray[0].VarList[1]);}
            if (strcmp(myargs[0],"history")==0 ){show();}
            if (p->pipeNum>0){
                fork_pipes(p->pipeNum,p->CommArray);
            }
            else{

                if(p->boolInfile){
                    infile = open(p->inFile, O_RDONLY);
                    dup2(infile,fileno(stdin));
                    close(infile);
                }
                if (p->boolOutfile){
                    outfile = open(p->outFile,O_CREAT|O_WRONLY|O_TRUNC, S_IRWXU);
                    dup2(outfile,fileno(stdout));
                    close(outfile);
                }
                
                if(p->boolBackground ==1)
                        {
                            if (counter<=10){
                            //addJob(counter,myargs[0],counter);
                            jobs[counter].pid = counter;
                                char *cmd_ = malloc(100);
                                strcpy(jobs[counter].cmdname,cmd_);
                                printf("*******%s******",jobs[counter].cmdname);
                                counter++;
                            }
                            else{
                                printf("unable to add: limit exceeded");
                            }
                            // signal(SIGCHLD , handler);
                        }
                 else
				{
					if (execvp(myargs[0], myargs) == -1)
					{
						perror("execvp");
						exit(1);
					};
				}
            }
        
            
        }else {
            if(p->boolBackground ==1)
                    {
                        printf("%s",prompt);
                        //waitpid(rc , &status , WNOHANG);
                    }
            else{
                wait(NULL);

            }
        }
    }

    return 0;
}