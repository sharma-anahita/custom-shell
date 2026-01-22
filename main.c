#include "shelly.h"

//This is a simple shell implementation in C
//what this shell is going to support
//execute the inbuilt commands
//execute the external commands
//we will also manage path through our shell
//and error handling will also be done
//we will first have to create a loop that will keep on running until the user wants to exit

void shell_loop(char** env){
    //1. setup  input and loop
    char* input = NULL;
    size_t inputLen = 0;
    char** args;
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
        //right now we will make the inbuild command function stub
        
    }
}
int main(int argc, char** argv,char** env){
    //env will help us get the environment variables
    (void)argc;
    (void)argv; // we don't need them
    printf("Welcome to this simple shell!\n");
    shell_loop(env);
}