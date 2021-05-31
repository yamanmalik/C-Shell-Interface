//Yaman Malik, 217290636, EECS3221 Winter 2021

#include <stdio.h> 
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>
#define MAX_LINE 80 /* The maximum length command */

int main(void) {
    char *args[MAX_LINE/2 + 1]; /* command line arguments */ 
    int should_run = 1; /* flag to determine when to exit program */
   int counter = 0; //counts number of commands (for use in the history !! command)
    
    while (should_run){ 

         
    //variable declrations
    int input = 0;
    int output = 0;
    int inputRedirect = 0;
    int outputRedirect = 0;
    int pipeRedirect = 0;
    char filename[20];
    char *pipeCommand[80];
    int ampersand = 0;
    int fd[2];
    

    printf("mysh:~$ "); 
    fflush(stdout);
    char temp[80];
    char history[80];
    char *tokens;
        

        
        /** After reading user input, the steps are:  
         * (1) fork a child process using fork()  
         * (2) the child process will invoke execvp()  
         * (3) parent will invoke wait() unless command included & 
         */



//receiving input

        fgets(temp,80,stdin);
       


//exit 
        if(strcmp(temp, "exit\n") == 0 ){
            should_run = 0;
            exit(0);
            break;
        }

//history function, repeats lasat command if counter > 0

        if(strstr(temp,"!!") != NULL){
            if(counter == 0){
                printf("No commands in history.\n");
                break;
            }
            strcpy(temp, history);
            printf("%s", temp);
        }
        else{
            strcpy(history, temp);
            counter++;
        }


//converting the input into tokens (breaking the input by space and newline into seperate strings and placing them in args)
        tokens = strtok(temp, " \n");

        int i = 0;
    

        while (tokens != NULL){
           
            args[i] = tokens;
            
            i++;
            tokens = strtok(NULL, " \n");
        }   
        args[i]= '\0';

//redirection loops

//this loop checks if < was entered
    int j = 0;

    while(args[j] != NULL){

        if(strcmp(args[j],"<") == 0 ){
            
            strcpy(filename,args[j+1]);
            args[j+1] = NULL;
            args[j] = NULL;
            inputRedirect = 1;

        }
        j++;
    }

    //this loop checks if > was entered
    int k = 0;
    while(args[k] != NULL){
        if(strcmp(args[k],">") == 0 ){
            
            strcpy(filename,args[k+1]);
            args[k+1] = NULL;
            args[k] = NULL;
            outputRedirect = 1;

        }
        k++;
    }


//this loop checks if an ampersand was entered
        int h = 0;

           while(args[h] != NULL){
            if(strcmp(args[h],"&") == 0 ){
                
               
                ampersand = 1;
                args[h] = NULL;
                break;

            }
            h++;
        }

        
   
//for cd command

    char cdtemp[80];

    if(strcmp(args[0], "cd") == 0) {

         chdir(args[1]);
         //print new directory after changing
         printf("%s\n", getcwd(cdtemp, 100));

    }       


//in case pipe operator is entered, this loop seperates the commands before and after the pipe and puts them into args and pipeCommand
    int z = 0;
    int y = 0;
    while(args[z] != NULL){
        if(strcmp(args[z],"|") == 0 ){
            
            args[z] = NULL;
            z++;
            while(args[z]!= NULL){
                
                pipeCommand[y] = args[z];
                    
                args[z] = NULL;
                    
                y++;
                z++;
            }    
            pipeRedirect = 1;
        }
        if(args[z] == NULL){
                break;
        }
        z++;
    }

  //null terminating array of commands after pipe character
        pipeCommand[y]= '\0';





//proccesses



//initializing pipe fd, (if pipe command is entered at shell)

        if (pipeRedirect == 1) {
            if (pipe(fd) == -1) {
                return 1;
            }
        }


       //creating first child and parent
        pid_t processid = fork();

        if(processid < 0){
            printf("Error occurred in the process!\n");

        }

        //child process
        else if (processid == 0){

            //if output redirect
            if(outputRedirect == 1){
                
                output = open(filename, O_WRONLY | O_CREAT, 0777);
                dup2(output,STDOUT_FILENO);
                close(output);
            }

            //if input redirect
            if(inputRedirect == 1){
                 
                input = open(filename, O_RDONLY);
                dup2(input,STDIN_FILENO);
                close(input);
            }

            //pipe 
            if(pipeRedirect == 1){
                
                dup2(fd[1], STDOUT_FILENO);
                close(fd[0]);
                close(fd[1]);     
            }
            
            execvp(args[0], args);
            
        }

        //parent process
        else{

            if(pipeRedirect ==1){

                //second fork

                  pid_t processid2 = fork();

                if(processid2 < 0){
                    printf("Error occurred in the second fork process!\n");
                }

            //second child

                if(processid2 == 0){

                    dup2(fd[0], STDIN_FILENO);
                    close(fd[0]);
                    close(fd[1]);
                    
                    execvp(pipeCommand[0], pipeCommand);
                }

                  close(fd[0]);
                  close(fd[1]);

                waitpid(processid, NULL, 0);
                waitpid(processid2, NULL, 0);
                break;

            }

            //waiting if no & is entered into terminal
            if(ampersand == 0){
                wait(NULL);

            }
        }

    }

return 0;
}
//end of program