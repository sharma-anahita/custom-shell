#ifndef SHELLY_H
#define SHELLY_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define MAX_INPUT_SIZE 1024

char **input_parser(char *input);
void free_tokens(char **tokens);

int my_strcmp(const char *str1, const char *str2);

/* builtins */
void command_cd(char **args, char *inputDirectory);
void command_pwd(char **args);
void command_which(char **args, char **env);
void command_echo(char **args);
void command_help(char **args);
void command_env(char **args, char **env);
void command_set(char **args, char **env);
void command_unset(char **args, char **env);

/* external */
void command_external(char **args, char **env);

#endif
