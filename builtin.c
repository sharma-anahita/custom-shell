#include "shelly.h"

void command_cd(char **args, char **inputDirectory,char** env)
{
    // supports cd, cd ~ (home directory), cd <path> , cd ..
    // to handle later : cd -(go to previous directory), permission denied handling
    //  will use chdir to change directory

    if (args[1] == NULL)
    {
        // print the current working directory
        printf("%s\n", *inputDirectory);
    }
    else
    {
        // change the directory to args[1]
        if (my_strcmp(args[1], "~") == 0)
        {
            // change to home directory
            char *path = my_getenv("HOME",env);
            if (!path)
            {
                path = my_getenv("USERPROFILE",env); // for Windows compatibility
            }

            if (!path)
            {
                fprintf(stderr, "shelly: HOME not set\n");
                return;
            }
            if (chdir(path) != 0)
            {
                perror("shelly");
            }
        }
        else
        {
            if (chdir(args[1]) != 0)
            {
                perror("command cd");
            }
        }
        free(*inputDirectory);
        *inputDirectory = getcwd(NULL, 0);
    }
    printf("Changed directory to: %s\n", *inputDirectory);
    return;
}

void command_pwd(char **args)
{
    (void) args;
    char *buf = NULL;
    size_t size = 0;
    buf = getcwd(buf, size);
    if (buf != NULL)
    {
        printf("%s\n", buf);
        free(buf);
    }
    else
    {
        perror("getcwd() error");
    }
}

void command_which(char **args, char **env);

void command_echo(char **args,char ** env)
{
    /*
     * command_echo
     * -------------
     * Implements the `echo` built-in command for the shell.
     *
     * Supported features:
     *  - Prints all arguments passed after `echo`
     *  - Supports the `-n` flag to suppress the trailing newline
     *  - Expands environment variables prefixed with `$`
     *    (e.g., `$HOME`, `$PATH`)
     *
     * Behavior notes:
     *  - Arguments are printed in order, separated by spaces
     *  - If an environment variable is not found, nothing is printed
     *  - A newline is printed by default unless `-n` is specified
     *
     * This function does not handle parsing of quotes or escape sequences;
     * those responsibilities belong to the input parser.
     *
     * Parameters:
     *  - args: NULL-terminated array of command arguments
     *
     * This implementation mimics basic POSIX shell `echo` behavior
     * while remaining intentionally simple and readable.
     */
    bool newLine = true;
    if (args[1] == NULL)
    {
        printf("No arguments given to args");
    }
    else
    {
        size_t i = 1;
        if (args[i] && my_strcmp(args[i], "-n") == 0)
        {

            // disable printing a newline
            newLine = false;
            i++;
        }
        for (; args[i] != NULL; i++)
        {
            if (args[i] && args[i][0] == '$')
            {
                char *variable = args[i] + 1;
                
                char *value = my_getenv(variable,env);

                if (!value && my_strcmp(variable, "HOME") == 0)
                {
                    value = my_getenv("USERPROFILE",env);
                }
                if (value)
                {
                    printf("%s", value);
                }
                if (args[i + 1])
                {
                    printf(" ");
                }
            }
            else
            { 
                    printf("%s", args[i]);
                    
                    if (args[i+1] != NULL)
                    {
                        printf(" ");
                    }
                
            }
        }
    }
    if (newLine)
    {
        printf("\n");
        printf("\n");
    }
    else
    {
        printf("\n");
    }
    fflush(stdout);
    return;
}


void command_help(char **args);
void command_env(char **args, char **env){
    //implment version with arguments too
    (void)args;
    for(size_t i = 0;env[i];i++){
        printf("%s",env[i]);
        printf("\n");
    }
}
void command_set(char **args, char **env);
void command_unset(char **args, char **env);