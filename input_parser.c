#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define MAX_TOKENS 64

char **input_parser(char *input)
{
    char **tokens = malloc(sizeof(char *) * MAX_TOKENS);
    if (!tokens) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    size_t i = 0;          // index in input string
    size_t token_idx = 0;  // index in tokens array

    while (input[i] != '\0') {

        /* 1️⃣ Skip leading whitespace */
        while (isspace((unsigned char)input[i])) {
            i++;
        }

        if (input[i] == '\0') {
            break;
        }

        /* 2️⃣ Mark start of token */
        size_t start = i;

        /* 3️⃣ Find end of token */
        while (input[i] != '\0' && !isspace((unsigned char)input[i])) {
            i++;
        }

        size_t len = i - start;

        /* 4️⃣ Allocate memory for token (+1 for '\0') */
        tokens[token_idx] = malloc(len + 1);
        if (!tokens[token_idx]) {
            perror("malloc");
            exit(EXIT_FAILURE);
        }

        /* 5️⃣ Copy token */
        memcpy(tokens[token_idx], &input[start], len);
        tokens[token_idx][len] = '\0';

        token_idx++;

        /* 6️⃣ Prevent overflow */
        if (token_idx >= MAX_TOKENS - 1) {
            break;
        }
    }

    /* 7️⃣ NULL-terminate argv */
    tokens[token_idx] = NULL;

    return tokens;
}
