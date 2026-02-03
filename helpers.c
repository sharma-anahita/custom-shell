#include "shelly.h"

char *my_getenv(const char *name, char **env)
{
    if (name == NULL || env == NULL)
    {
        return NULL;
    }
    size_t nameLen = my_strLen(name);
    for (size_t i = 0; env[i]; i++)
    {
        if (my_strncmp(env[i], name, nameLen, false) == 0 && env[i][nameLen] == '=')
        {

            return &env[i][nameLen + 1];
        }
    }
    return NULL;
}

void free_tokens(char **tokens)
{
    for (int i = 0; tokens[i]; i++)
    {
        free(tokens[i]);
    }
    free(tokens);
}

int my_strLen(const char *name)
{
    int len = 0;
    while (*name)
    {
        name++;
        len++;
    }
    return len;
}
int my_strchr(const char *deli, char ch)
{
    char *st = deli;
    while (*st)
    {
        if (*st == ch)
            return 1;
        st++;
    }
    return 0;
}
char *my_strtok(char *path, const char *deli, char **save)
{
    if (path)
    {
        *save = path;
    }
    if (*save == NULL)
    {
        return NULL;
    }
    char *start = *save;

    while (*start && my_strchr(deli, *start))
        start++;

    if (*start == '\0')
    {
        *save = NULL;
        return NULL;
    }

    char *end = start;
    while (*end && my_strchr(deli, *end) == 0)
        end++;

    // start.....end -> file path

    if (*end)
    {
        *end = '\0';
        *save = end + 1;
    }
    else
    {
        *save = NULL;
    }
    return start;
}
int my_strncmp(const char *str1, const char *str2, size_t n, bool caseSensitive)
{
    int i = 0;
    while (i < n && str1[i] && str2[i])
    {
        char ch1 = str1[i];
        char ch2 = str2[i];
        if (caseSensitive)
        {
            if (ch1 != ch2)
            {
                return (unsigned char)str1[i] - (unsigned char)str2[i];
            }
        }
        else
        {
            if (tolower(ch1) != tolower(ch2))
            {
                return (unsigned char)str1[i] - (unsigned char)str2[i];
            }
        }

        i++;
    }
    if (i == n)
        return 0;
    return (unsigned char)str1[i] - (unsigned char)str2[i];
}
char *my_strdup(const char *src)
{
    if (src == NULL)
        return NULL;
    size_t len = my_strLen(src);
    char *dest = malloc(len + 1);
    if (dest == NULL)
        return NULL;
    for (int i = 0; i < len; i++)
    {
        dest[i] = src[i];
    }
    dest[len] = '\0';
    return dest;
}
char *my_strconcat(char *str1, char *str2)
{

    int n = my_strLen(str1);
    int m = my_strLen(str2);
    char *new1 = malloc(n + m + 1);
    for (int i = 0; i < n; i++)
    {
        new1[i] = str1[i];
    }
    for (int i = 0; i < m; i++)
    {
        new1[i + n] = str2[i];
    }
    new1[m + n] = '\0';
    return new1;
}
int my_strcmp(const char *str1, const char *str2)
{
    while (*str1 && (*str1 == *str2))
    {
        str1++;
        str2++;
    }
    return *(const unsigned char *)str1 - *(const unsigned char *)str2;
}

char *find_command(char *path, char *command)
{
    char *pathcopy = my_strdup(path);
    if (!pathcopy)
        return NULL;
    printf("checked null pathcopy\n");
    char *save = NULL;
    char *dir = my_strtok(pathcopy, PATH_DELIMS, &save);

    while (dir)
    {
        // printf("%s\n",dir);
        char fullpath[MAX_PATH_LENGTH];

#ifdef _WIN32
        snprintf(fullpath, sizeof(fullpath), "%s\\%s", dir, command);
#else
        snprintf(fullpath, sizeof(fullpath), "%s/%s", dir, command);
#endif
        
// printf("%s\n",fullpath);

#ifdef _WIN32
        if (_access(fullpath, 0) == 0)
        {
            free(pathcopy);
            printf("found the executable");
            return my_strdup(fullpath);
        }
#else
        if (access(fullpath, X_OK) == 0)
        {
            free(pathcopy);
            printf("found the executable");
            return my_strdup(fullpath);
        }
#endif

        dir = my_strtok(NULL, PATH_DELIMS, &save);
        
    }
    free(pathcopy);
    return NULL;
}

char *find_command_in_path(char *command, char **env)
{
    if (!command || !env)
        return NULL; 
    char *path = my_getenv("PATH", env);
    if (!path)
        return NULL; 

    // find just the string
    char *ans = find_command(path, command);
    if (ans)
        return ans; 
    // find whole executables
    char *pathext = my_getenv("PATHEXT", env);
    if(pathext==NULL){
        return NULL;
    } 
    // tokenise them
    char *pathextcpy = my_strdup(pathext);
    
    if(pathextcpy==NULL){
        return NULL;
    } 

    char *save = NULL;
    char *token = my_strtok(pathextcpy, ";", &save);
    while (token)
    {
        char *commandnew = my_strconcat(command, token);
        printf("%s", commandnew);
        ans = find_command(path, commandnew);
        if (ans)
            return ans;
        token = my_strtok(NULL, ";", &save);
        free(commandnew);
    }

    return NULL;
}
