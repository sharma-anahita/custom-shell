#include "shelly.h"
#include <sys/wait.h>

//this is where we'll execute the external commands

//we'll have to create a new process for them
//will return something to tell if it was sucessfull or not

const char* strsignal(int sig){
    char* signal = strsignal(SIGSEGV);
    return signal;
}
void command_external(char **args, char **env){
    //1st arg is the external command 
    //we'll have to make a new process
    int pid = fork();
    if(pid==-1){
        perror("fork couldn't create a new process");
    }
    else if(pid==0){
        //this is the new child process with access to same resources has been created
        //here we need to execute our command
        //find the executable and execute
        char* pathofexe = find_command_in_path(args[0],env);
        if(!pathofexe){
            printf("Invalid Command\n");
            perror("path not found of the command requested");
        }
        execve(pathofexe,args,env);
        perror("Execution failed");
        exit(1);
        // now i have to execute this 
        
    }
    else{
        int status; //to get info about child
        waitpid(pid,&status,0) ; // i need to block parent till child executes completely
        if(WIFEXITED(status)){
            printf("Program exited sucessfully %d\n",WEXITSTATUS(status));
        }
        else if(WIFSIGNALED(status)){
            
            int sig = WSTOPSIG(status);
            char* signal  = strsignal(sig);
            printf("Program killed by a signal %s",signal);
            if(WCOREDUMP(status)){
                printf(" (core dumped) \n");
            }
        }
        else if (WIFSTOPPED(status)){
            int sig = WSTOPSIG(status);
            char* signal  = strsignal(sig);
            printf("Process was stopped by a signal %d",signal);

        }
        else{
            
        }
    }
}