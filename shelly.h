#ifndef SHELLY_H
#define SHELLY_H
#ifdef _WIN32
#define PATH_DELIMS ";"
#else
#define PATH_DELIMS ":"
#endif

// for increasing portability
#ifdef _WIN32
#include <direct.h>   // _chdir, _getcwd
#else
#include <unistd.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#define MAX_INPUT_SIZE 1024
#define MAX_PATH_LENGTH 4096
char **input_parser(char *input);
void free_tokens(char **tokens);

// string functions
int my_strcmp(const char *str1, const char *str2);
int my_strLen(const char *name);
int my_strncmp(const char *str1, const char *str2, size_t n, bool caseSensitive);
char *my_strdup(const char *src);
char* my_strconcat(char* str1,char*str2);

/* builtins */
void command_cd(char **args, char **inputDirectory,char** env);
void command_pwd(char **args);
void command_which(char **args, char **env);
void command_echo(char **args,char** env);
void command_help(char **args);
void command_env(char **args, char **env);
void command_set(char **args, char **env);
void command_unset(char **args, char **env);

char* my_getenv(const char* name,char ** env);
/* external */
void command_external(char **args, char **env);

#endif
