#include "shelly.h"

void command_cd(char **args, char **inputDirectory, char **env)
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
            char *path = my_getenv("HOME", env);
            if (!path)
            {
                path = my_getenv("USERPROFILE", env); // for Windows compatibility
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
    (void)args;
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

char *find_command_in_path(char *command, char **env);
void command_which(char **args, char **env)
{
    // add functionality for options -l specifically

    (void)env;
    // find the location of the executable
    char *builtins[] = {
        "pwd",
        "cd",
        "echo",
        "env",
        "help",
        "setenv",
        "unset",
        "which",
        NULL};

    if (args[1] == NULL)
    {
        printf("which [options] [--] COMMAND [...]");
    }
    else
    {
        // make it a loop for all the commands as args
        size_t len = my_strLen(args[1]);
        bool found = false;
        for (int i = 0; i < 8; i++)
        {
            if (my_strncmp(builtins[i], args[1], len, true) == 0)
            {
                printf("this one's a built-in command");
                found = true;
            }
        }
        if (!found)
        {
            printf("this one's a external command, searching in env $PATH\n");
            char *command = find_command_in_path(args[1], env);
            printf("%s ", command);
        }
    }
    printf("\n");
    return;
}

void command_echo(char **args, char **env)
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
    /*
     * Environment variable lookup in Shelly is case-insensitive.
     *
     * This is a deliberate design choice to improve usability and to
     * align better with Windows-style environments, where variables
     * such as PATH, Path, and path are treated equivalently.
     *
     * This behavior differs from traditional POSIX shells.
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

                char *value = my_getenv(variable, env);

                if (!value && my_strcmp(variable, "HOME") == 0)
                {
                    value = my_getenv("USERPROFILE", env);
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

                if (args[i + 1] != NULL)
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

void command_help(char **args)
{
    (void)args;
    printf("HI");
}
int env_len(char **env)
{
    int i = 0;
    while (env[i])
        i++;
    return i;
}
void command_env(char **args, char **env)
{
    // implment version with arguments too
    (void)args;
    for (size_t i = 0; env[i]; i++)
    {
        printf("%s", env[i]);
        printf("\n");
    }
}
static int key_len(const char *s)
{
    int i = 0;
    while (s[i] && s[i] != '=')
        i++;
    return i;
}
char **command_set(char **args, char ***env)
{
    int envc = env_len(*env);
    int found = 0;

    // allocate space: old env + possible new var + NULL
    char **newenv = malloc(sizeof(char *) * (envc + 2));
    if (!newenv)
    {
        perror("malloc");
        return NULL;
    }

    // deep copy env
    for (int i = 0; i < envc; i++)
    {
        newenv[i] = my_strdup((*env)[i]);
        if (!newenv[i])
        {
            perror("strdup");
            return NULL;
        }
    }
    newenv[envc] = NULL;

    // no variable provided
    if (args[1] == NULL)
        return newenv;

    int keylen = key_len(args[1]);

    // search and update
    for (int i = 0; i < envc; i++)
    {
        if (strncmp(newenv[i], args[1], keylen) == 0 &&
            newenv[i][keylen] == '=')
        {
            free(newenv[i]);

            // case: VAR=value
            if (args[2] == NULL)
            {
                newenv[i] = my_strdup(args[1]);
            }
            // case: VAR value
            else
            {
                int varlen = my_strLen(args[1]);
                int vallen = my_strLen(args[2]);

                newenv[i] = malloc(varlen + vallen + 2);
                if (!newenv[i])
                {
                    perror("malloc");
                    return NULL;
                }

                snprintf(newenv[i], varlen + vallen + 2,
                         "%s=%s", args[1], args[2]);
            }

            found = 1;
            break;
        }
    }

    // not found → append new variable (only if value exists)
    if (!found && args[2] != NULL)
    {
        int varlen = my_strLen(args[1]);
        int vallen = my_strLen(args[2]);

        newenv[envc] = malloc(varlen + vallen + 2);
        if (!newenv[envc])
        {
            perror("malloc");
            return NULL;
        }

        snprintf(newenv[envc], varlen + vallen + 2,
                 "%s=%s", args[1], args[2]);

        newenv[envc + 1] = NULL;
    }

    return newenv;
}


int findInSet(char* set[],char* key,int keyLen){
    int indx = 0;
    while(set[indx]){
        if(my_strncmp(key,set[indx],keyLen,false)==0 && (key[keyLen]=='=') && set[indx][keyLen]=='\0') return 1;
        indx+=1;
    }
    return 0;
}
char** command_unset(char **args, char ***env){
    printf("yo entered\n");
    int env_Len = env_len(*env);
    char** newenv = malloc((env_Len+1)*sizeof(char*));
    //try to fin the variable
    //if found remove it 
    //add all others into the newenv, except this one
    if(!newenv){
        perror("could'nt allocate newenv inside unset command\n");
        return NULL;
    }
    if(!args[1]){
        printf("No arguments given to unset_env function\n");
        return NULL;
    }
    int count = 0;
    while(args[count]){
        count++;
    }
    
    char* set[count];
    int indx=0;
    while(args[indx+1]){
        set[indx] = args[indx+1];
        printf("%s\n",set[indx]);
        indx++;
    }
    set[indx]= NULL;
    //set has all args
    int newenvIndx = 0;
    for (int i = 0; i < env_Len; i++)
    {
        //see if this one is to be deleted
        int keyLen = key_len((*env)[i]);
        if(findInSet(set,(*env)[i],keyLen)==0){
            
            newenv[newenvIndx] = my_strdup((*env)[i]); 
            printf("%s\n",newenv[newenvIndx]);
            newenvIndx++;
        } 
        free((*env)[i]);
    }
    
    newenv[newenvIndx]=NULL;
    return newenv;
}