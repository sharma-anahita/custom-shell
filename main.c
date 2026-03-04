#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "shelly.h"

/* ─────────────────────────────────────────────
 *  Execute a single built-in or external command.
 *  Redirections on a single command are handled
 *  by command_external → apply_redirections.
 *  Built-ins get redirections applied in the
 *  parent process directly (simple open/dup2).
 * ───────────────────────────────────────────── */
static void apply_redirections_parent(Command *cmd,
                                      int *saved_in,
                                      int *saved_out,
                                      int *saved_err)
{
    /* save originals so we can restore after built-in runs */
    *saved_in  = dup(STDIN_FILENO);
    *saved_out = dup(STDOUT_FILENO);
    *saved_err = dup(STDERR_FILENO);

    if (cmd->redirect_in)
    {
        int fd = open(cmd->redirect_in, O_RDONLY);
        if (fd < 0) perror(cmd->redirect_in);
        else { dup2(fd, STDIN_FILENO); close(fd); }
    }
    if (cmd->redirect_out)
    {
        int flags = O_WRONLY | O_CREAT | (cmd->append ? O_APPEND : O_TRUNC);
        int fd    = open(cmd->redirect_out, flags, 0644);
        if (fd < 0) perror(cmd->redirect_out);
        else { dup2(fd, STDOUT_FILENO); close(fd); }
    }
    if (cmd->redirect_err)
    {
        int fd = open(cmd->redirect_err, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd < 0) perror(cmd->redirect_err);
        else { dup2(fd, STDERR_FILENO); close(fd); }
    }
}

static void restore_std_fds(int saved_in, int saved_out, int saved_err)
{
    dup2(saved_in,  STDIN_FILENO);  close(saved_in);
    dup2(saved_out, STDOUT_FILENO); close(saved_out);
    dup2(saved_err, STDERR_FILENO); close(saved_err);
}

int shell_builtin_execute(Command *cmd, char ***env, char **inputDirectory)
{
    char **args = cmd->args;

    /* apply redirections in the parent for built-ins */
    int sin, sout, serr;
    apply_redirections_parent(cmd, &sin, &sout, &serr);

    if (my_strcmp(args[0], "cd") == 0)
    {
        command_cd(args, inputDirectory, *env);
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
        command_echo(args, *env);
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
        char **newenv = command_set(args, env);
        if (newenv) { free(*env); *env = newenv; }
    }
    else if (my_strcmp(args[0], "unset") == 0)
    {
        char **newenv = command_unset(args, env);
        if (newenv) { free(*env); *env = newenv; }
        else perror("unset: allocation failed");
    }
    else
    {
        /* external single command — restore fds first,
           externals.c handles its own redirections in the child */
        restore_std_fds(sin, sout, serr);
        command_external(args, *env);
        return 0;
    }

    restore_std_fds(sin, sout, serr);
    return 0;
}

/* ─────────────────────────────────────────────
 *  Main shell loop
 * ───────────────────────────────────────────── */
#define INPUT_BUFSIZE 1024

void shell_loop(char **env)
{
    char  input[INPUT_BUFSIZE];
    char *inputDirectory = getcwd(NULL, 0);

    while (1)
    {
        printf("(My_Shelly)>%s ", inputDirectory);
        fflush(stdout);

        if (!fgets(input, sizeof(input), stdin))
        {
            printf("\n");
            break; /* EOF: Ctrl+D / Ctrl+Z */
        }

        /* ── lex ── */
        int    token_count = 0;
        Token *tokens      = tokenize(input, &token_count); /* BUG FIX: & */
        if (!tokens) { perror("tokenize"); exit(EXIT_FAILURE); }

        /* ── parse ── */
        int      cmd_count = 0;
        Command *commands  = parse_commands(tokens, token_count, &cmd_count);
        free_tokens_new(tokens, token_count);

        if (!commands) { perror("parse_commands"); exit(EXIT_FAILURE); }

        /* skip empty input */
        if (cmd_count == 0 || commands[0].args[0] == NULL)
        {
            free_commands(commands, cmd_count);
            continue;
        }

        /* ── dispatch ── */
        if (cmd_count == 1)
        {
            /* single command — may have redirections but no pipes */
            shell_builtin_execute(&commands[0], &env, &inputDirectory);
        }
        else
        {
            /* pipeline: built-ins inside a pipeline run in a child,
               so their env/dir changes won't affect the parent shell —
               that's standard shell behaviour. */
            execute_pipeline(commands, cmd_count, env);
        }

        free_commands(commands, cmd_count);
    }

    free(inputDirectory);
}

int main(int argc, char **argv, char **env)
{
    setvbuf(stdout, NULL, _IOLBF, 0);
    (void)argc;
    (void)argv;
    printf("Welcome to this simple shell!\n");
    shell_loop(env);
}