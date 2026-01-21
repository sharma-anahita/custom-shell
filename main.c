#include <stdio.h>
#include <stdbool.h>
#include <string.h>

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
    while(1){
        if(getline(&input,&inputLen,stdin) == -1){
            break; //EOF reached
            //use Ctrl+Z to send EOF in Windows
            //use Ctrl+D to send EOF in Linux/Mac
        }
        printf("%s",input);
        //2. parse the input
    }
}
int main(int argc, char** argv,char** env){
    //env will help us get the environment variables
    (void)argc;
    (void)argv; // we don't need them
    printf("Welcome to this simple shell!\n");
    shell_loop(env);
}