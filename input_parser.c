#include "shelly.h"

char** input_parser(char* input){
    //allocate space for maximum number of arguments
    size_t bufferSize  =MAX_INPUT_SIZE; //max input size for 1 argument
    char** tokens = malloc(bufferSize * sizeof(char *)); //what is the meaning of this line?
    //this line allocates memory for an array of character pointers (strings)

    //Error handling for malloc

    if(!tokens){
        perror("malloc failed to allocate memory to input parser");
        exit(EXIT_FAILURE);
        //TELLS the OS that the program has failed
    }

    //if we have allocated memory succesfully
    //now we will tokenize the input string

    char* token = NULL;
    size_t token_len = 0;
    size_t position = 0;
    //tokenize the input based on whitespace?
    for(int i = 0;input[i];i++){
        token = &input[position];
        token_len = 0;
        while(input[i] && input[i] != ' '){
            //not a whitespace and a valid character
            token_len++;
            i++;
        }
        tokens[position] = malloc(token_len*sizeof(char));

        //see if any space was allocated
        if(!tokens[position]){
            perror("mallloc failed to allocate space to token");
            exit(EXIT_FAILURE);
            //but what if it is a whitespace?
            //we will handle that later
        }

        //space has been allocated 
        //now put the chars in the token
        for(int j = 0;j<token_len;j++){
            //copy the characters            
            tokens[position][j] = token[j];
            
        }
        tokens[position][token_len] = '\0'; //null terminate the string
        position++;
    }
    
}