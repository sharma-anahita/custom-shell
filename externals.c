
#include "shelly.h"
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/wait.h>
#include <unistd.h>
#endif

//this is where we'll execute the external commands

//we'll have to create a new process for them
//will return something to tell if it was sucessfull or not
 

void command_external(char **args, char **env) {
#ifdef _WIN32
    // Windows implementation using CreateProcess
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    // Build command line string
    char *cmdline = my_strdup("");

    for (int i = 0; args[i] != NULL; i++) {
        char *tmp = my_strconcat(cmdline, args[i]);
        free(cmdline);
        cmdline = tmp;

        tmp = my_strconcat(cmdline, " ");
        free(cmdline);
        cmdline = tmp;
    }

    // Remove trailing space
    size_t len = my_strLen(cmdline);
    if (len > 0 && cmdline[len - 1] == ' ') {
        cmdline[len - 1] = '\0';
    }

    if (!CreateProcessA(
            NULL,           // No module name (use command line)
            cmdline,        // Command line
            NULL,           // Process handle not inheritable
            NULL,           // Thread handle not inheritable
            FALSE,          // Set handle inheritance to FALSE
            0,              // No creation flags
            NULL,           // Use parent's environment block
            NULL,           // Use parent's starting directory 
            &si,            // Pointer to STARTUPINFO structure
            &pi)            // Pointer to PROCESS_INFORMATION structure
        ) {
        printf("CreateProcess failed (%lu).\n", GetLastError());
        return;
    }

    // Wait until child process exits.
    WaitForSingleObject(pi.hProcess, INFINITE);

    // Get exit code
    DWORD exitCode;
    if ((GetExitCodeProcess(pi.hProcess, &exitCode)) && exitCode!=0) {
        printf("Program exited with code %lu\n", exitCode);
    }

    // Close process and thread handles. 
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
#else
    // Unix/Linux implementation using fork/exec
    int pid = fork();
    if (pid == -1) {
        perror("fork couldn't create a new process");
    } else if (pid == 0) {
        char* pathofexe = find_command_in_path(args[0], env);
        if (!pathofexe) {
            printf("Invalid Command\n");
            perror("path not found of the command requested");
            exit(1);
        }
        execve(pathofexe, args, env);
        perror("Execution failed");
        exit(1);
    } else {
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) {
            printf("Program exited sucessfully %d\n", WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            int sig = WTERMSIG(status);
            char* signal = strsignal(sig);
            printf("Program killed by a signal %s", signal);
            if (WCOREDUMP(status)) {
                printf(" (core dumped) \n");
            }
        } else if (WIFSTOPPED(status)) {
            int sig = WSTOPSIG(status);
            char* signal = strsignal(sig);
            printf("Process was stopped by a signal %s", signal);
        }
    }
#endif
}