#include "shelly.h"

/* ═══════════════════════════════════════════════════════════════
 *  Redirection helpers for built-in commands.
 *
 *  Built-ins run in the PARENT process, so we must save/restore
 *  the real stdin/stdout/stderr around them.
 *
 *  On Windows we use the CRT fd layer (_dup / _dup2) which sits
 *  on top of Win32 HANDLEs and is consistent with _open/fopen.
 * ═══════════════════════════════════════════════════════════════ */

#ifdef _WIN32
#define SH_DUP(fd) _dup(fd)
#define SH_DUP2(src, dst) _dup2((src), (dst))
#define SH_CLOSE(fd) _close(fd)
#define SH_OPEN3(p, f, m) _open((p), (f) | _O_BINARY, (m))
#else
#define SH_DUP(fd) dup(fd)
#define SH_DUP2(src, dst) dup2((src), (dst))
#define SH_CLOSE(fd) close(fd)
#define SH_OPEN3(p, f, m) open((p), (f), (m))
#endif

static void apply_redirections_parent(Command *cmd,
                                      int *saved_in,
                                      int *saved_out,
                                      int *saved_err)
{
    *saved_in = SH_DUP(STDIN_FILENO);
    *saved_out = SH_DUP(STDOUT_FILENO);
    *saved_err = SH_DUP(STDERR_FILENO);

    if (cmd->redirect_in)
    {
        int fd = SH_OPEN3(cmd->redirect_in, O_RDONLY, 0);
        if (fd < 0)
            perror(cmd->redirect_in);
        else
        {
            SH_DUP2(fd, STDIN_FILENO);
            SH_CLOSE(fd);
        }
    }
    if (cmd->redirect_out)
    {
        int flags = O_WRONLY | O_CREAT | (cmd->append ? O_APPEND : O_TRUNC);
        int fd = SH_OPEN3(cmd->redirect_out, flags, 0644);
        if (fd < 0)
            perror(cmd->redirect_out);
        else
        {
            SH_DUP2(fd, STDOUT_FILENO);
            SH_CLOSE(fd);
        }
    }
    if (cmd->redirect_err)
    {
        int fd = SH_OPEN3(cmd->redirect_err, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd < 0)
            perror(cmd->redirect_err);
        else
        {
            SH_DUP2(fd, STDERR_FILENO);
            SH_CLOSE(fd);
        }
    }
}

static void restore_std_fds(int saved_in, int saved_out, int saved_err)
{
    SH_DUP2(saved_in, STDIN_FILENO);
    SH_CLOSE(saved_in);
    SH_DUP2(saved_out, STDOUT_FILENO);
    SH_CLOSE(saved_out);
    SH_DUP2(saved_err, STDERR_FILENO);
    SH_CLOSE(saved_err);
}

/* ═══════════════════════════════════════════════════════════════
 *  Dispatch a single command (built-in or external).
 * ═══════════════════════════════════════════════════════════════ */
int shell_builtin_execute(Command *cmd, char ***env, char **inputDirectory)
{
    char **args = cmd->args;

    int sin_fd, sout_fd, serr_fd;
    apply_redirections_parent(cmd, &sin_fd, &sout_fd, &serr_fd);

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
        if (newenv)
        {
            free(*env);
            *env = newenv;
        }
    }
    else if (my_strcmp(args[0], "unset") == 0)
    {
        char **newenv = command_unset(args, env);
        if (newenv)
        {
            free(*env);
            *env = newenv;
        }
        else
            perror("unset: allocation failed");
    }
    else
    {
        /* External: restore fds first — externals.c opens its own
           redirections inside the child process / CreateProcess */
        restore_std_fds(sin_fd, sout_fd, serr_fd);
        command_external(args, *env);
        return 0;
    }

    restore_std_fds(sin_fd, sout_fd, serr_fd);
    return 0;
}

/* ═══════════════════════════════════════════════════════════════
 *  Main shell loop
 * ═══════════════════════════════════════════════════════════════ */
#define INPUT_BUFSIZE 1024

void shell_loop(char **env)
{
    char input[INPUT_BUFSIZE];
    char *inputDirectory = getcwd(NULL, 0);

    while (1)
    {
        char prompt[MAX_PATH_LENGTH + 16];
        snprintf(prompt, sizeof(prompt), "(My_Shelly)>%s ", inputDirectory);
        char *line = shell_readline(prompt);
        if (!line)
            break;
        strncpy(input, line, INPUT_BUFSIZE - 1);
        // if (!fgets(input, sizeof(input), stdin))
        // {
        //     printf("\n");
        //     break; /* EOF: Ctrl+D on Unix, Ctrl+Z on Windows */
        // }

        /* strip trailing newline / \r\n (Windows fgets includes \r) */
        int len = my_strLen(input);
        while (len > 0 && (input[len - 1] == '\n' || input[len - 1] == '\r'))
        {
            input[--len] = '\0';
        }

        /* lex */
        int token_count = 0;
        Token *tokens = tokenize(input, &token_count);
        if (!tokens)
        {
            perror("tokenize");
            exit(EXIT_FAILURE);
        }

        /* parse */
        int cmd_count = 0;
        Command *commands = parse_commands(tokens, token_count, &cmd_count);
        free_tokens_new(tokens, token_count);

        if (!commands)
        {
            perror("parse_commands");
            exit(EXIT_FAILURE);
        }

        if (cmd_count == 0 || commands[0].args[0] == NULL)
        {
            free_commands(commands, cmd_count);
            continue;
        }

        /* dispatch */
        if (cmd_count == 1)
            shell_builtin_execute(&commands[0], &env, &inputDirectory);
        else
            execute_pipeline(commands, cmd_count, env);

        free_commands(commands, cmd_count);
    }

    free(inputDirectory);
}

/* ═══════════════════════════════════════════════════════════════
 *  Entry point
 *
 *  POSIX:   main(int, char**, char**)  -- third arg is envp
 *  Windows: envp is also supported by MinGW/MSVC as an extension
 * ═══════════════════════════════════════════════════════════════ */
int main(int argc, char **argv, char **env)
{
    (void)argc;
    (void)argv;

    /* line-buffered stdout on both platforms */
    setvbuf(stdout, NULL, _IOLBF, 0);

    printf("Welcome to this simple shell!\n");
    shell_loop(env);
    return 0;
}