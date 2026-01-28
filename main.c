#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "shelly.h"


// This is a simple shell implementation in C
// what this shell is going to support
// execute the inbuilt commands
// execute the external commands
// we will also manage path through our shell
// and error handling will also be done
// we will first have to create a loop that will keep on running until the user wants to exit

int shell_builtin_execute(char **args, char ***env, char **inputDirectory)
{
    // this function will execute the inbuilt commands
    // cd, pwd,whihch, echo,exit,help,env,set,unset,etc
    printf("Executing built in command: %s\n", args[0]);
    if (my_strcmp(args[0], "cd") == 0)
    {
        printf("you called cd\n");
        command_cd(args, inputDirectory,*env);
    }
    else if (my_strcmp(args[0], "pwd") == 0)
    {
        command_pwd(args);
    }
    else if (my_strcmp(args[0], "which") == 0)
    {
        command_which(args, *env);
    }
    else if (my_strcmp(args[0], "echo") == 0)
    {
        command_echo(args,*env);
    }
    else if (my_strcmp(args[0], "help") == 0)
    {
        command_help(args);
    }
    else if (my_strcmp(args[0], "exit") == 0 ||
             my_strcmp(args[0], "quit") == 0)
    {
        exit(EXIT_SUCCESS);
    }
    else if (my_strcmp(args[0], "env") == 0)
    {
        command_env(args, *env);
    }
    else if (my_strcmp(args[0], "set") == 0)
    {
        char** newenv = command_set(args, env);
        if(newenv){
            free(*env);
            *env = newenv;
        }
    }
    else if (my_strcmp(args[0], "unset") == 0)
    {
        command_unset(args, *env);
    }
    else
    {
        command_external(args, *env);
    }

    return 0;
}
#define INPUT_BUFSIZE 1024

//binary commands : ls, cat, grep, ps, top, etc  // will call them external commands
void shell_loop(char **env)
{
    char input[INPUT_BUFSIZE];
    char **args;
    char *inputDirectory = getcwd(NULL, 0);

    while (1) {
        printf("(My_Shelly)>%s ", inputDirectory);
        fflush(stdout);

        if (!fgets(input, sizeof(input), stdin)) {
            printf("\n");
            break;   // EOF (Ctrl+D / Ctrl+Z)
        }

        args = input_parser(input);

        if (args[0] == NULL) {
            free_tokens(args);
            continue;
        }

        shell_builtin_execute(args, &env, &inputDirectory);
        free_tokens(args);
    }

    free(inputDirectory);
}

int main(int argc, char **argv, char **env)
{
    
setvbuf(stdout, NULL, _IOLBF, 0);
    // env will help us get the environment variables
    (void)argc;
    (void)argv; // we don't need them
    printf("Welcome to this simple shell!\n");
    shell_loop(env);
}