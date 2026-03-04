#include "shelly.h"
#include <sys/wait.h>
#include <fcntl.h>

/* ─────────────────────────────────────────────
 *  strsignal forward declaration (POSIX)
 * ───────────────────────────────────────────── */
#ifndef _GNU_SOURCE
const char *strsignal(int sig);
#endif

/* ─────────────────────────────────────────────
 *  Report how a child process exited
 * ───────────────────────────────────────────── */
static void report_status(int status)
{
    if (WIFEXITED(status))
    {
        int code = WEXITSTATUS(status);
        if (code != 0)
            fprintf(stderr, "shelly: process exited with status %d\n", code);
    }
    else if (WIFSIGNALED(status))
    {
        int sig = WTERMSIG(status);
        fprintf(stderr, "shelly: killed by signal %d (%s)%s\n",
                sig, strsignal(sig),
                WCOREDUMP(status) ? " (core dumped)" : "");
    }
    else if (WIFSTOPPED(status))
    {
        int sig = WSTOPSIG(status);
        fprintf(stderr, "shelly: stopped by signal %d (%s)\n",
                sig, strsignal(sig));
    }
}

/* ─────────────────────────────────────────────
 *  Apply redirections inside a child process.
 *  Returns 0 on success, -1 on error.
 * ───────────────────────────────────────────── */
static int apply_redirections(Command *cmd)
{
    if (cmd->redirect_in)
    {
        int fd = open(cmd->redirect_in, O_RDONLY);
        if (fd < 0) { perror(cmd->redirect_in); return -1; }
        if (dup2(fd, STDIN_FILENO) < 0)  { perror("dup2 stdin");  return -1; }
        close(fd);
    }

    if (cmd->redirect_out)
    {
        int flags = O_WRONLY | O_CREAT | (cmd->append ? O_APPEND : O_TRUNC);
        int fd    = open(cmd->redirect_out, flags, 0644);
        if (fd < 0) { perror(cmd->redirect_out); return -1; }
        if (dup2(fd, STDOUT_FILENO) < 0) { perror("dup2 stdout"); return -1; }
        close(fd);
    }

    if (cmd->redirect_err)
    {
        int fd = open(cmd->redirect_err,
                      O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd < 0) { perror(cmd->redirect_err); return -1; }
        if (dup2(fd, STDERR_FILENO) < 0) { perror("dup2 stderr"); return -1; }
        close(fd);
    }

    return 0;
}

/* ─────────────────────────────────────────────
 *  Single external command  (no pipes)
 * ───────────────────────────────────────────── */
void command_external(char **args, char **env)
{
    pid_t pid = fork();
    if (pid < 0)
    {
        perror("fork");
        return;
    }

    if (pid == 0)
    {
        /* child */
        char *path = find_command_in_path(args[0], env);
        if (!path)
        {
            fprintf(stderr, "shelly: %s: command not found\n", args[0]);
            exit(127);
        }
        execve(path, args, env);
        perror("execve");
        exit(1);
    }

    /* parent */
    int status;
    waitpid(pid, &status, 0);
    report_status(status);
}

/* ─────────────────────────────────────────────
 *  Pipeline execution
 *
 *  For N commands we need N-1 pipes.
 *
 *  Layout (example, N=3):
 *
 *    cmd[0] ──pipe[0]──► cmd[1] ──pipe[1]──► cmd[2]
 *
 *  Each pipe is an int[2]: [0]=read end, [1]=write end.
 *
 *  Child i:
 *    - If not first:  dup2(pipe[i-1][0], STDIN)
 *    - If not last:   dup2(pipe[i][1],   STDOUT)
 *    - Close ALL pipe fds (it now holds them via stdin/stdout)
 *    - Apply any explicit redirections from the Command struct
 *    - execve
 *
 *  Parent:
 *    - Close ALL pipe fds after forking all children
 *    - waitpid for every child
 * ───────────────────────────────────────────── */
void execute_pipeline(Command *commands, int cmd_count, char **env)
{
    int n_pipes = cmd_count - 1;

    /* allocate pipe array */
    int (*pipes)[2] = malloc(sizeof(int[2]) * n_pipes);
    if (!pipes) { perror("malloc pipes"); return; }

    /* create all pipes up front */
    for (int i = 0; i < n_pipes; i++)
    {
        if (pipe(pipes[i]) < 0)
        {
            perror("pipe");
            /* close pipes already opened */
            for (int j = 0; j < i; j++) { close(pipes[j][0]); close(pipes[j][1]); }
            free(pipes);
            return;
        }
    }

    pid_t *pids = malloc(sizeof(pid_t) * cmd_count);
    if (!pids) { perror("malloc pids"); free(pipes); return; }

    /* ── fork one child per command ── */
    for (int i = 0; i < cmd_count; i++)
    {
        pids[i] = fork();

        if (pids[i] < 0)
        {
            perror("fork");
            /* best-effort: kill siblings already forked */
            for (int j = 0; j < i; j++) kill(pids[j], SIGTERM);
            break;
        }

        if (pids[i] == 0)
        {
            /* ── CHILD i ── */

            /* wire up stdin from previous pipe */
            if (i > 0)
            {
                if (dup2(pipes[i-1][0], STDIN_FILENO) < 0)
                { perror("dup2 pipe stdin"); exit(1); }
            }

            /* wire up stdout to next pipe */
            if (i < n_pipes)
            {
                if (dup2(pipes[i][1], STDOUT_FILENO) < 0)
                { perror("dup2 pipe stdout"); exit(1); }
            }

            /* close ALL pipe fds — child inherited them all */
            for (int p = 0; p < n_pipes; p++)
            {
                close(pipes[p][0]);
                close(pipes[p][1]);
            }

            /* apply explicit file redirections (overrides pipe if set) */
            if (apply_redirections(&commands[i]) < 0)
                exit(1);

            /* find and exec */
            char *path = find_command_in_path(commands[i].args[0], env);
            if (!path)
            {
                fprintf(stderr, "shelly: %s: command not found\n",
                        commands[i].args[0]);
                exit(127);
            }
            execve(path, commands[i].args, env);
            perror("execve");
            exit(1);
        }
        /* parent continues to next iteration */
    }

    /* ── PARENT: close all pipe fds ── */
    for (int i = 0; i < n_pipes; i++)
    {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    /* ── PARENT: wait for all children ── */
    for (int i = 0; i < cmd_count; i++)
    {
        int status;
        waitpid(pids[i], &status, 0);
        report_status(status);
    }

    free(pids);
    free(pipes);
}