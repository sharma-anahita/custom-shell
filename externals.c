#include "shelly.h"

/* ═══════════════════════════════════════════════════════════════
 *  externals.c  -  Execute external commands and pipelines
 *
 *  Two completely separate implementations behind #ifdef:
 *    - POSIX  : fork / execve / pipe / dup2 / waitpid
 *    - Windows: CreateProcess / CreatePipe / HANDLE
 * ═══════════════════════════════════════════════════════════════ */

/* ───────────────────────────────────────────────────────────────
 *  Shared helper: build a single command-line string from argv
 *  (needed on Windows where CreateProcess takes a flat string).
 *  Caller must free the result.
 * ─────────────────────────────────────────────────────────────── */
static char *args_to_cmdline(char **args) __attribute__((unused));
static char *args_to_cmdline(char **args)
{
    /* calculate total length */
    size_t total = 1; /* null terminator */
    for (int i = 0; args[i]; i++)
        total += my_strLen(args[i]) + 3; /* quotes + space */

    char *cmdline = malloc(total);
    if (!cmdline)
    {
        perror("malloc");
        return NULL;
    }
    cmdline[0] = '\0';

    for (int i = 0; args[i]; i++)
    {
        if (i > 0)
            strcat(cmdline, " ");
        /* wrap in quotes to handle spaces in paths/args */
        strcat(cmdline, "\"");
        strcat(cmdline, args[i]);
        strcat(cmdline, "\"");
    }
    return cmdline;
}

/*windows */
#ifdef _WIN32

/* ---------------------------------------------------------------
 *  Open a file for redirection and return its HANDLE.
 * --------------------------------------------------------------- */
static HANDLE open_redirect_handle(const char *filename, bool is_input, bool append)
{
    DWORD access = is_input ? GENERIC_READ : GENERIC_WRITE;
    DWORD share = FILE_SHARE_READ | FILE_SHARE_WRITE;
    DWORD disp;

    if (is_input)
        disp = OPEN_EXISTING;
    else if (append)
        disp = OPEN_ALWAYS;
    else
        disp = CREATE_ALWAYS;

    SECURITY_ATTRIBUTES sa = {
        .nLength = sizeof(SECURITY_ATTRIBUTES),
        .lpSecurityDescriptor = NULL,
        .bInheritHandle = TRUE /* child must inherit this */
    };

    HANDLE h = CreateFileA(filename, access, share, &sa, disp,
                           FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE)
    {
        fprintf(stderr, "shelly: cannot open '%s': error %lu\n",
                filename, GetLastError());
        return INVALID_HANDLE_VALUE;
    }

    if (append && !is_input)
        SetFilePointer(h, 0, NULL, FILE_END);

    return h;
}

/* ---------------------------------------------------------------
 *  Spawn one process with explicit stdin/stdout/stderr handles.
 *  Returns the process HANDLE (caller must CloseHandle it after
 *  WaitForSingleObject), or NULL on failure.
 * --------------------------------------------------------------- */
static HANDLE spawn_process(char **args, char **env,
                            HANDLE h_stdin,
                            HANDLE h_stdout,
                            HANDLE h_stderr)
{
    char *path = find_command_in_path(args[0], env);
    if (!path)
    {
        fprintf(stderr, "shelly: %s: command not found\n", args[0]);
        return NULL;
    }

    char *cmdline = args_to_cmdline(args);
    if (!cmdline)
    {
        free(path);
        return NULL;
    }

    /* Build environment block: "KEY=VALUE\0KEY=VALUE\0\0" */
    /* Count total size */
    size_t env_size = 1; /* final double-null */
    for (int i = 0; env[i]; i++)
        env_size += my_strLen(env[i]) + 1;

    char *env_block = malloc(env_size);
    if (!env_block)
    {
        free(path);
        free(cmdline);
        return NULL;
    }
    char *p = env_block;
    for (int i = 0; env[i]; i++)
    {
        size_t len = my_strLen(env[i]);
        memcpy(p, env[i], len + 1);
        p += len + 1;
    }
    *p = '\0'; /* double-null terminator */

    STARTUPINFOA si;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdInput = h_stdin;
    si.hStdOutput = h_stdout;
    si.hStdError = h_stderr;

    PROCESS_INFORMATION pi;
    ZeroMemory(&pi, sizeof(pi));

    BOOL ok = CreateProcessA(
        path,      /* lpApplicationName  */
        cmdline,   /* lpCommandLine      */
        NULL,      /* process security   */
        NULL,      /* thread security    */
        TRUE,      /* bInheritHandles    */
        0,         /* dwCreationFlags    */
        env_block, /* lpEnvironment      */
        NULL,      /* lpCurrentDirectory */
        &si,
        &pi);

    free(path);
    free(cmdline);
    free(env_block);

    if (!ok)
    {
        fprintf(stderr, "shelly: CreateProcess failed: error %lu\n",
                GetLastError());
        return NULL;
    }

    CloseHandle(pi.hThread); /* we only need the process handle */
    return pi.hProcess;
}

/* ---------------------------------------------------------------
 *  Report exit code of a completed child process.
 * --------------------------------------------------------------- */
static void report_process(HANDLE hProc)
{
    DWORD code = 0;
    GetExitCodeProcess(hProc, &code);
    if (code != 0)
        fprintf(stderr, "shelly: process exited with code %lu\n", code);
    CloseHandle(hProc);
}

/* ---------------------------------------------------------------
 *  Single external command  (no pipes)
 * --------------------------------------------------------------- */
void command_external(char **args, char **env)
{
    HANDLE hProc = spawn_process(args, env,
                                 GetStdHandle(STD_INPUT_HANDLE),
                                 GetStdHandle(STD_OUTPUT_HANDLE),
                                 GetStdHandle(STD_ERROR_HANDLE));
    if (!hProc)
        return;
    WaitForSingleObject(hProc, INFINITE);
    report_process(hProc);
}

/* ---------------------------------------------------------------
 *  Pipeline  cmd[0] | cmd[1] | ... | cmd[N-1]
 *
 *  Windows anonymous pipes: CreatePipe(&read, &write, &sa, 0)
 *
 *  We must mark the handle the child does NOT use as
 *  non-inheritable, otherwise the child holds an extra copy of
 *  the write end open and the reader never gets EOF.
 * --------------------------------------------------------------- */
void execute_pipeline(Command *commands, int cmd_count, char **env)
{
    int n_pipes = cmd_count - 1;

    /* Arrays of pipe read/write handles */
    HANDLE *pipe_read = malloc(sizeof(HANDLE) * n_pipes);
    HANDLE *pipe_write = malloc(sizeof(HANDLE) * n_pipes);
    if (!pipe_read || !pipe_write)
    {
        perror("malloc");
        free(pipe_read);
        free(pipe_write);
        return;
    }

    SECURITY_ATTRIBUTES sa = {
        .nLength = sizeof(SECURITY_ATTRIBUTES),
        .lpSecurityDescriptor = NULL,
        .bInheritHandle = TRUE};

    /* Create all pipes */
    for (int i = 0; i < n_pipes; i++)
    {
        if (!CreatePipe(&pipe_read[i], &pipe_write[i], &sa, 0))
        {
            fprintf(stderr, "shelly: CreatePipe failed: error %lu\n",
                    GetLastError());
            for (int j = 0; j < i; j++)
            {
                CloseHandle(pipe_read[j]);
                CloseHandle(pipe_write[j]);
            }
            free(pipe_read);
            free(pipe_write);
            return;
        }
        /* The parent's copy of each end must NOT be inherited by
           children that don't need it — mark them non-inheritable */
        SetHandleInformation(pipe_read[i], HANDLE_FLAG_INHERIT, 0);
        SetHandleInformation(pipe_write[i], HANDLE_FLAG_INHERIT, 0);
    }

    HANDLE *proc_handles = malloc(sizeof(HANDLE) * cmd_count);
    if (!proc_handles)
    {
        perror("malloc");
        for (int i = 0; i < n_pipes; i++)
        {
            CloseHandle(pipe_read[i]);
            CloseHandle(pipe_write[i]);
        }
        free(pipe_read);
        free(pipe_write);
        return;
    }

    for (int i = 0; i < cmd_count; i++)
    {
        /* determine stdin handle for this child */
        HANDLE h_in;
        if (i == 0)
        {
            if (commands[i].redirect_in)
                h_in = open_redirect_handle(commands[i].redirect_in, true, false);
            else
                h_in = GetStdHandle(STD_INPUT_HANDLE);
        }
        else
        {
            /* make the read end of previous pipe inheritable for this child */
            SetHandleInformation(pipe_read[i - 1], HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
            h_in = pipe_read[i - 1];
        }

        /* determine stdout handle for this child */
        HANDLE h_out;
        if (i == n_pipes) /* last command */
        {
            if (commands[i].redirect_out)
                h_out = open_redirect_handle(commands[i].redirect_out,
                                             false, commands[i].append);
            else
                h_out = GetStdHandle(STD_OUTPUT_HANDLE);
        }
        else
        {
            /* make the write end of this pipe inheritable for this child */
            SetHandleInformation(pipe_write[i], HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
            h_out = pipe_write[i];
        }

        /* stderr */
        HANDLE h_err;
        if (commands[i].redirect_err)
            h_err = open_redirect_handle(commands[i].redirect_err, false, false);
        else
            h_err = GetStdHandle(STD_ERROR_HANDLE);

        proc_handles[i] = spawn_process(commands[i].args, env,
                                        h_in, h_out, h_err);

        /* After spawning, re-mark handles non-inheritable and close our
           copy of the pipe ends we passed to the child — the child owns
           them now via its STARTUPINFO */
        if (i > 0)
        {
            SetHandleInformation(pipe_read[i - 1], HANDLE_FLAG_INHERIT, 0);
            CloseHandle(pipe_read[i - 1]);
        }
        if (i < n_pipes)
        {
            SetHandleInformation(pipe_write[i], HANDLE_FLAG_INHERIT, 0);
            CloseHandle(pipe_write[i]);
        }
        /* Close redirect handles we opened */
        if (commands[i].redirect_in && i == 0)
            CloseHandle(h_in);
        if (commands[i].redirect_out && i == n_pipes)
            CloseHandle(h_out);
        if (commands[i].redirect_err)
            CloseHandle(h_err);
    }

    /* Wait for all children */
    for (int i = 0; i < cmd_count; i++)
    {
        if (proc_handles[i])
        {
            WaitForSingleObject(proc_handles[i], INFINITE);
            report_process(proc_handles[i]); /* also CloseHandle */
        }
    }

    free(pipe_read);
    free(pipe_write);
    free(proc_handles);
}

// POSIX
#else /* !_WIN32 */

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

/* Apply file redirections inside a child process. */
static int apply_redirections(Command *cmd)
{
    if (cmd->redirect_in)
    {
        int fd = open(cmd->redirect_in, O_RDONLY);
        if (fd < 0)
        {
            perror(cmd->redirect_in);
            return -1;
        }
        if (dup2(fd, STDIN_FILENO) < 0)
        {
            perror("dup2 stdin");
            return -1;
        }
        close(fd);
    }
    if (cmd->redirect_out)
    {
        int flags = O_WRONLY | O_CREAT | (cmd->append ? O_APPEND : O_TRUNC);
        int fd = open(cmd->redirect_out, flags, 0644);
        if (fd < 0)
        {
            perror(cmd->redirect_out);
            return -1;
        }
        if (dup2(fd, STDOUT_FILENO) < 0)
        {
            perror("dup2 stdout");
            return -1;
        }
        close(fd);
    }
    if (cmd->redirect_err)
    {
        int fd = open(cmd->redirect_err, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd < 0)
        {
            perror(cmd->redirect_err);
            return -1;
        }
        if (dup2(fd, STDERR_FILENO) < 0)
        {
            perror("dup2 stderr");
            return -1;
        }
        close(fd);
    }
    return 0;
}

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

    int status;
    waitpid(pid, &status, 0);
    report_status(status);
}

void execute_pipeline(Command *commands, int cmd_count, char **env)
{
    int n_pipes = cmd_count - 1;

    int (*pipes)[2] = malloc(sizeof(int[2]) * n_pipes);
    if (!pipes)
    {
        perror("malloc pipes");
        return;
    }

    for (int i = 0; i < n_pipes; i++)
    {
        if (pipe(pipes[i]) < 0)
        {
            perror("pipe");
            for (int j = 0; j < i; j++)
            {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }
            free(pipes);
            return;
        }
    }

    pid_t *pids = malloc(sizeof(pid_t) * cmd_count);
    if (!pids)
    {
        perror("malloc pids");
        free(pipes);
        return;
    }

    for (int i = 0; i < cmd_count; i++)
    {
        pids[i] = fork();
        if (pids[i] < 0)
        {
            perror("fork");
            for (int j = 0; j < i; j++)
                kill(pids[j], SIGTERM);
            break;
        }

        if (pids[i] == 0)
        {
            /* wire stdin */
            if (i > 0)
            {
                if (dup2(pipes[i - 1][0], STDIN_FILENO) < 0)
                {
                    perror("dup2 pipe stdin");
                    exit(1);
                }
            }
            /* wire stdout */
            if (i < n_pipes)
            {
                if (dup2(pipes[i][1], STDOUT_FILENO) < 0)
                {
                    perror("dup2 pipe stdout");
                    exit(1);
                }
            }
            /* close all pipe fds in child */
            for (int p = 0; p < n_pipes; p++)
            {
                close(pipes[p][0]);
                close(pipes[p][1]);
            }
            /* explicit redirections override pipe wiring */
            if (apply_redirections(&commands[i]) < 0)
                exit(1);

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
    }

    /* parent: close all pipe fds */
    for (int i = 0; i < n_pipes; i++)
    {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    /* parent: wait for all children */
    for (int i = 0; i < cmd_count; i++)
    {
        int status;
        waitpid(pids[i], &status, 0);
        report_status(status);
    }

    free(pids);
    free(pipes);
}

#endif /* _WIN32 */