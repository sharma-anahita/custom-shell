#ifndef SHELLY_H
#define SHELLY_H

/* ═══════════════════════════════════════════════════════════════
 *  PLATFORM DETECTION & COMPATIBILITY LAYER
 *
 *  All platform differences are isolated here so that every
 *  other .c file can be written once and compiled on both
 *  Windows (MinGW / MSYS2) and POSIX (Linux, macOS).
 * ═══════════════════════════════════════════════════════════════ */

#ifdef _WIN32
    /* ── Windows ─────────────────────────────────────────── */
    #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
    #endif
    #include <windows.h>    /* CreateProcess, HANDLE, pipes  */
    #include <io.h>         /* _access, _open_osfhandle      */
    #include <direct.h>     /* _getcwd, _chdir               */
    #include <process.h>    /* _getpid                       */

    /* map POSIX names -> Windows equivalents */
    #define getcwd(buf, sz)   _getcwd((buf), (sz))
    #define chdir(path)       _chdir(path)
    #define access(p, m)      _access((p), (m))
    #define PATH_DELIMS       ";"
    #define DIR_SEP           '\\'
    #define EXE_EXT           ".exe"

    /* fd-level constants missing on Windows */
    #define STDIN_FILENO   0
    #define STDOUT_FILENO  1
    #define STDERR_FILENO  2

    /* O_* flags for _open / _sopen */
    #ifndef O_RDONLY
    #define O_RDONLY   _O_RDONLY
    #endif
    #ifndef O_WRONLY
    #define O_WRONLY   _O_WRONLY
    #endif
    #ifndef O_CREAT
    #define O_CREAT    _O_CREAT
    #endif
    #ifndef O_TRUNC
    #define O_TRUNC    _O_TRUNC
    #endif
    #ifndef O_APPEND
    #define O_APPEND   _O_APPEND
    #endif
    #ifndef O_BINARY
    #define O_BINARY   _O_BINARY
    #endif

#else
    /* ── POSIX (Linux / macOS) ───────────────────────────── */
    #define _GNU_SOURCE
    #include <unistd.h>
    #include <sys/wait.h>
    #include <sys/types.h>
    #include <signal.h>

    #define PATH_DELIMS  ":"
    #define DIR_SEP      '/'
    #define EXE_EXT      ""
#endif

/* ── Headers needed on both platforms ──────────────────────── */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <fcntl.h>
#include <errno.h>

/* ═══════════════════════════════════════════════════════════════
 *  CONSTANTS
 * ═══════════════════════════════════════════════════════════════ */
#define MAX_INPUT_SIZE   1024
#define MAX_PATH_LENGTH  4096
#define MAX_TOKENS       128

/* ═══════════════════════════════════════════════════════════════
 *  LEXER TYPES
 * ═══════════════════════════════════════════════════════════════ */
typedef enum {
    TOKEN_WORD,
    TOKEN_PIPE,             /*  |   */
    TOKEN_REDIRECT_OUT,     /*  >   */
    TOKEN_REDIRECT_APPEND,  /*  >>  */
    TOKEN_REDIRECT_IN,      /*  <   */
    TOKEN_REDIRECT_ERR      /*  2>  */
} ShellTokenType;

typedef struct {
    ShellTokenType  type;
    char           *value;   /* only set for TOKEN_WORD */
} Token;

/* ═══════════════════════════════════════════════════════════════
 *  COMMAND  -  one segment of a pipeline
 * ═══════════════════════════════════════════════════════════════ */
typedef struct {
    char **args;          /* NULL-terminated argv   */
    int    arg_count;
    char  *redirect_in;   /* filename or NULL        */
    char  *redirect_out;  /* filename or NULL        */
    char  *redirect_err;  /* filename or NULL        */
    bool   append;        /* true when >>            */
} Command;

/* ═══════════════════════════════════════════════════════════════
 *  PARSER / LEXER API
 * ═══════════════════════════════════════════════════════════════ */
Token   *tokenize(char *input, int *out_count);
Command *parse_commands(Token *tokens, int token_count, int *out_cmd_count);
void     free_tokens_new(Token *tokens, int count);
void     free_commands(Command *cmds, int count);

char   **input_parser(char *input);
void     free_tokens(char **tokens);

/* ═══════════════════════════════════════════════════════════════
 *  STRING HELPERS
 * ═══════════════════════════════════════════════════════════════ */
int   my_strcmp(const char *str1, const char *str2);
int   my_strLen(const char *name);
int   my_strncmp(const char *str1, const char *str2, size_t n, bool caseSensitive);
char *my_strdup(const char *src);
char *my_strconcat(char *str1, char *str2);
int   my_strchr(const char *deli, char ch);
char *my_strtok(char *path, const char *deli, char **save);
char *my_getenv(const char *name, char **env);
char *shell_readline(const char *prompt);
/* ═══════════════════════════════════════════════════════════════
 *  BUILT-IN COMMANDS
 * ═══════════════════════════════════════════════════════════════ */
void   command_cd(char **args, char **inputDirectory, char **env);
void   command_pwd(char **args);
void   command_which(char **args, char **env);
void   command_echo(char **args, char **env);
void   command_help(char **args);
void   command_env(char **args, char **env);
char **command_set(char **args, char ***env);
char **command_unset(char **args, char ***env);

/* ═══════════════════════════════════════════════════════════════
 *  EXTERNAL COMMAND EXECUTION
 * ═══════════════════════════════════════════════════════════════ */
void  command_external(char **args, char **env);
void  execute_pipeline(Command *commands, int cmd_count, char **env);
char *find_command_in_path(char *command, char **env);
// to implement tab completion
void term_raw_enable(void);
void term_raw_disable(void);
int  term_read_char(void);   // returns one char 
#endif /* SHELLY_H */