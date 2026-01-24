#include "shelly.h" 

 

void command_which(char **args, char **env)
{
    printf("which not implemented\n");
}
 

void command_help(char **args)
{
    printf("help not implemented\n");
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
int my_strLen(const char* name){
    int len = 0;
    while(*name){
        name++;
        len++;
    }
    return len;
}
int my_strncmp(const char* str1,const char* str2,size_t n){
    int i =0; 
    while(i<n && str1[i] && str2[i]){
        if(str1[i] != str2[i]){
            return (unsigned char)str1[i] - (unsigned char)str2[i];
        }
        i++;
        
    }   
    if(i==n) return 0;
    return (unsigned char)str1 - (unsigned char)str2;
}   
char* my_getenv(const char* name,char ** env){
    if(name==NULL || env==NULL){
        return NULL;
        
    }
    size_t nameLen = my_strLen(name);
    for (size_t i = 0; env[i]; i++)
    {
        if(my_strncmp(env[i],name,nameLen)==0 && env[i][nameLen]=='='){
            return &env[i][nameLen+1];
        }
    }
    
}
