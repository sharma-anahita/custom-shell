#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#define MAX_INPUT_SIZE 1024

char** input_parser(char* input);
int shell_builtin_execute(char** args, char** env, char* inputDirectroy);

int command_cd(char ** args,char* inputDirectory);
int command_pwd(char ** args);;
int command_which(char ** args, char** env);
int command_echo(char** args);
int command_help(char** args);
int command_exit(char** args);
int command_env(char** args, char** env);
int command_set(char** args, char** env);   
int command_unset(char** args, char** env);