#include "shelly.h"

//This is a simple shell implementation in C
//what this shell is going to support
//execute the inbuilt commands
//execute the external commands
//we will also manage path through our shell
//and error handling will also be done
//we will first have to create a loop that will keep on running until the user wants to exit
int shell_builtin_execute(char** args, char** env, char* inputDirectroy){
    //this function will execute the inbuilt commands
    //cd, pwd,whihch, echo,exit,help,env,set,unset,etc
    if(stcmp(args[0],"cd") == 0){
        command_cd(args,inputDirectroy);
        
    }
    else if(stcmp(args[0],'pwd')==0){
        command_pwd(args);
    }
    else if(stcmp(args[0],'which')==0){
        command_which(args,env);
        //exlpain which to me
        //The which command in Unix-like operating systems is used to locate the executable file associated with a given command by searching through the directories listed in the user's PATH environment variable.
        //does user need to provide env here?
        //yes because which needs to search through the PATH variable

    }
    else if(stcmp(args[0],'echo ')==0){
        command_echo(args);
    }
    else if(stcmp(args[0],'help')==0){
        command_help(args);
        //does help need args?
        //yes because help can provide help for specific commands
        //env?
        //no
        //why
        //because help does not need to search through environment variables
    }
    else if(stcmp(args[0],'exit')==0 || stcmp(args[0],'quit')==0){
        command_exit(args);
    }
    else if(stcmp(args[0],'env')==0){
        command_env(args,env);
    }
    else if(stcmp(args[0],'set')==0){
        command_set(args,env);
    }
    else if(stcmp(args[0],'unset')==0){
        command_unset(args,env);
    }
    else{
        //external command
        //handle this later
        command_external(args,env);
    }
    
}
void shell_loop(char** env){
    //1. setup  input and loop
    char* input = NULL;
    size_t inputLen = 0;
    char** args;
    char * inputDirectory = getcwd(NULL,0); //get the current working directory
    
    while(1){
        printf("(shelly)> ");
        if(getline(&input,&inputLen,stdin) == -1){
            perror("getline");
            break; //EOF reached
            //use Ctrl+Z to send EOF in Windows
            //use Ctrl+D to send EOF in Linux/Mac
        } 
        //2. parse the input
        args = input_parser(input);
        for(int i = 0;args[i]!=NULL;i++){
            printf("Arg %d : %s\n",i,args[i]);
        }

        //3. execute the command
        //to execute the commands we will create a new function 
        //first we will check if the command is inbuilt or external
        //if inbuilt we will execute it directly
        //right now we will make the inbuild command function 
        shell_builtin_execute(args,env,inputDirectory);
        if(args[0]!=NULL){
            shell_builtin_execute(args,env,inputDirectory);
        }
    }
}
int main(int argc, char** argv,char** env){
    //env will help us get the environment variables
    (void)argc;
    (void)argv; // we don't need them
    printf("Welcome to this simple shell!\n");
    shell_loop(env);

}