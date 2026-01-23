#include "shelly.h"

void command_cd(char **args, char **inputDirectory)
{
    // will use chdir to change directory

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
            char *path = getenv("HOME"); 
            if (!path)
            {
                path = getenv("USERPROFILE"); // for Windows compatibility 
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
                perror("shelly");
            }
        }
        free(*inputDirectory);
        *inputDirectory = getcwd(NULL, 0);
    }
    printf("Changed directory to: %s\n", *inputDirectory);
    return;
}
void command_pwd(char **args);
void command_which(char **args, char **env);
void command_echo(char **args);
void command_help(char **args);
void command_env(char **args, char **env);
void command_set(char **args, char **env);
void command_unset(char **args, char **env);