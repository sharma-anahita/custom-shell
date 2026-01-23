#include "shelly.h" 

// void command_cd(char **args, char *cwd)
// {
//     printf("cd not implemented\n");
// }

void command_pwd(char **args)
{
    char buf[1024];
    if (getcwd(buf, sizeof(buf))) {
        printf("%s\n", buf);
    }
}

void command_which(char **args, char **env)
{
    printf("which not implemented\n");
}

void command_echo(char **args)
{
    for (int i = 1; args[i]; i++) {
        printf("%s ", args[i]);
    }
    printf("\n");
}

void command_help(char **args)
{
    printf("help not implemented\n");
}

void command_env(char **args, char **env)
{
    for (int i = 0; env[i]; i++) {
        printf("%s\n", env[i]);
    }
}

void command_set(char **args, char **env)
{
    printf("set not implemented\n");
}

void command_unset(char **args, char **env)
{
    printf("unset not implemented\n");
}

void command_external(char **args, char **env)
{
    printf("external command not implemented\n");
}

int my_strcmp(const char* str1, const char* str2){
    while(*str1 && (*str1==*str2)){
        str1++;
        str2++;
    }
    return *(const unsigned char*)str1 - *(const unsigned char*)str2;
    
}

void free_tokens(char **tokens)
{
    for (int i = 0; tokens[i]; i++) {
        free(tokens[i]);
    }
    free(tokens);
}
