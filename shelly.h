#ifndef SHELLY_H
#define SHELLY_H

#ifdef _WIN32
#define PATH_DELIMS ";"
#else
#define PATH_DELIMS ":"
#endif

#ifdef _WIN32
#include <direct.h>
#else
#include <unistd.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#define MAX_INPUT_SIZE   1024
#define MAX_PATH_LENGTH  4096
#define MAX_TOKENS       128

/* ─────────────────────────────────────────────
 *  Lexer types
 * ───────────────────────────────────────────── */

typedef enum {
    TOKEN_WORD,
    TOKEN_PIPE,            /*  |   */
    TOKEN_REDIRECT_OUT,    /*  >   */
    TOKEN_REDIRECT_APPEND, /*  >>  */
    TOKEN_REDIRECT_IN,     /*  <   */
    TOKEN_REDIRECT_ERR     /*  2>  */
} TokenType;

typedef struct {
    TokenType  type;
    char      *value;   /* only populated for TOKEN_WORD */
} Token;

/* ─────────────────────────────────────────────
 *  Parsed command (one segment between pipes)
 * ───────────────────────────────────────────── */

typedef struct {
    char **args;          /* NULL-terminated argv  */
    int    arg_count;
    char  *redirect_in;   /* filename, or NULL      */
    char  *redirect_out;  /* filename, or NULL      */
    char  *redirect_err;  /* filename, or NULL      */
    bool   append;        /* true when >>           */
} Command;

/* ─────────────────────────────────────────────
 *  Parser / lexer API
 * ───────────────────────────────────────────── */

/* New pipeline-aware API */
Token   *tokenize(char *input, int *out_count);
Command *parse_commands(Token *tokens, int token_count, int *out_cmd_count);
void     free_tokens_new(Token *tokens, int count);
void     free_commands(Command *cmds, int count);

/* Legacy single-command API (kept for compatibility) */
char   **input_parser(char *input);
void     free_tokens(char **tokens);

/* ─────────────────────────────────────────────
 *  String helpers
 * ───────────────────────────────────────────── */

int   my_strcmp(const char *str1, const char *str2);
int   my_strLen(const char *name);
int   my_strncmp(const char *str1, const char *str2, size_t n, bool caseSensitive);
char *my_strdup(const char *src);
char *my_strconcat(char *str1, char *str2);
int   my_strchr(const char *deli, char ch);
char *my_strtok(char *path, const char *deli, char **save);
char *my_getenv(const char *name, char **env);

/* ─────────────────────────────────────────────
 *  Built-in commands
 * ───────────────────────────────────────────── */

void   command_cd(char **args, char **inputDirectory, char **env);
void   command_pwd(char **args);
void   command_which(char **args, char **env);
void   command_echo(char **args, char **env);
void   command_help(char **args);
void   command_env(char **args, char **env);
char **command_set(char **args, char ***env);
char **command_unset(char **args, char ***env);

/* ─────────────────────────────────────────────
 *  External commands
 * ───────────────────────────────────────────── */

void  command_external(char **args, char **env);
char *find_command_in_path(char *command, char **env);
void execute_pipeline(Command *commands,int cmd_count,char** env);

#endif /* SHELLY_H */