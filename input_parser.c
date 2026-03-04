#include "shelly.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define MAX_TOKENS 128

/* ─────────────────────────────────────────────
 *  LEXER  –  raw input  →  Token[]
 * ───────────────────────────────────────────── */

Token *tokenize(char *input, int *out_count)
{
    Token *tokens = malloc(sizeof(Token) * MAX_TOKENS);
    if (!tokens) { perror("malloc"); exit(EXIT_FAILURE); }

    int count = 0;
    size_t i   = 0;

    while (input[i] != '\0')
    {
        /* skip whitespace */
        while (isspace((unsigned char)input[i])) i++;
        if (input[i] == '\0') break;

        Token t = { .type = TOKEN_WORD, .value = NULL };

        /* ── pipe ── */
        if (input[i] == '|')
        {
            t.type  = TOKEN_PIPE;
            t.value = NULL;
            i++;
        }

        /* ── stderr redirect  2> ── */
        else if (input[i] == '2' && input[i+1] == '>')
        {
            t.type  = TOKEN_REDIRECT_ERR;
            t.value = NULL;
            i += 2;
        }

        /* ── append  >> ── */
        else if (input[i] == '>' && input[i+1] == '>')
        {
            t.type  = TOKEN_REDIRECT_APPEND;
            t.value = NULL;
            i += 2;
        }

        /* ── redirect out  > ── */
        else if (input[i] == '>')
        {
            t.type  = TOKEN_REDIRECT_OUT;
            t.value = NULL;
            i++;
        }

        /* ── redirect in  < ── */
        else if (input[i] == '<')
        {
            t.type  = TOKEN_REDIRECT_IN;
            t.value = NULL;
            i++;
        }

        /* ── quoted word  "..."  or  '...' ── */
        else if (input[i] == '"' || input[i] == '\'')
        {
            char quote = input[i++];
            size_t start = i;
            while (input[i] && input[i] != quote) i++;
            size_t len = i - start;
            if (input[i] == quote) i++;          /* consume closing quote */

            t.type  = TOKEN_WORD;
            t.value = malloc(len + 1);
            if (!t.value) { perror("malloc"); exit(EXIT_FAILURE); }
            memcpy(t.value, &input[start], len);
            t.value[len] = '\0';
        }

        /* ── plain word ── */
        else
        {
            size_t start = i;
            while (input[i] && !isspace((unsigned char)input[i]) &&
                   input[i] != '|' && input[i] != '>' &&
                   input[i] != '<' && input[i] != '"' && input[i] != '\'')
            {
                /* treat  2>  as a single operator, so stop before '2' only
                   if the next char is '>'                                    */
                if (input[i] == '2' && input[i+1] == '>') break;
                i++;
            }
            size_t len = i - start;
            t.type  = TOKEN_WORD;
            t.value = malloc(len + 1);
            if (!t.value) { perror("malloc"); exit(EXIT_FAILURE); }
            memcpy(t.value, &input[start], len);
            t.value[len] = '\0';
        }

        tokens[count++] = t;
        if (count >= MAX_TOKENS - 1) break;
    }

    /* sentinel */
    tokens[count] = (Token){ .type = TOKEN_WORD, .value = NULL };
    *out_count = count;
    return tokens;
}

/* ─────────────────────────────────────────────
 *  FREE helpers
 * ───────────────────────────────────────────── */

void free_tokens_new(Token *tokens, int count)
{
    for (int i = 0; i < count; i++)
        if (tokens[i].value) free(tokens[i].value);
    free(tokens);
}

 
 /*  PARSER  –  Token[]  →  Command[] 
 *  A Command holds:
 *    args[]        – NULL-terminated argv
 *    redirect_in   – filename or NULL
 *    redirect_out  – filename or NULL
 *    redirect_err  – filename or NULL
 *    append        – true when >> was used  */

Command *parse_commands(Token *tokens, int token_count, int *out_cmd_count)
{
    /* upper bound: (token_count / 2) + 1 commands */
    int max_cmds = (token_count / 2) + 2;
    Command *cmds = malloc(sizeof(Command) * max_cmds);
    if (!cmds) { perror("malloc"); exit(EXIT_FAILURE); }

    int cmd_idx  = 0;
    int tok_idx  = 0;

    /* initialise first command slot */
    cmds[cmd_idx] = (Command){
        .args         = malloc(sizeof(char *) * MAX_TOKENS),
        .arg_count    = 0,
        .redirect_in  = NULL,
        .redirect_out = NULL,
        .redirect_err = NULL,
        .append       = false
    };

    while (tok_idx < token_count)
    {
        Token *t = &tokens[tok_idx];

        switch (t->type)
        {
        /* ── word → push onto current command's args ── */
        case TOKEN_WORD:
            cmds[cmd_idx].args[cmds[cmd_idx].arg_count++] = t->value;
            t->value = NULL;   /* ownership transferred */
            break;

        /* ── pipe → seal current command, start next ── */
        case TOKEN_PIPE:
            cmds[cmd_idx].args[cmds[cmd_idx].arg_count] = NULL;
            cmd_idx++;
            cmds[cmd_idx] = (Command){
                .args         = malloc(sizeof(char *) * MAX_TOKENS),
                .arg_count    = 0,
                .redirect_in  = NULL,
                .redirect_out = NULL,
                .redirect_err = NULL,
                .append       = false
            };
            break;

        /* ── redirect in:  < filename ── */
        case TOKEN_REDIRECT_IN:
            tok_idx++;
            if (tok_idx < token_count && tokens[tok_idx].type == TOKEN_WORD)
            {
                cmds[cmd_idx].redirect_in = tokens[tok_idx].value;
                tokens[tok_idx].value = NULL;
            }
            else
            {
                fprintf(stderr, "shelly: expected filename after '<'\n");
            }
            break;

        /* ── redirect out:  > filename ── */
        case TOKEN_REDIRECT_OUT:
            tok_idx++;
            if (tok_idx < token_count && tokens[tok_idx].type == TOKEN_WORD)
            {
                cmds[cmd_idx].redirect_out = tokens[tok_idx].value;
                tokens[tok_idx].value = NULL;
                cmds[cmd_idx].append = false;
            }
            else
            {
                fprintf(stderr, "shelly: expected filename after '>'\n");
            }
            break;

        /* ── append:  >> filename ── */
        case TOKEN_REDIRECT_APPEND:
            tok_idx++;
            if (tok_idx < token_count && tokens[tok_idx].type == TOKEN_WORD)
            {
                cmds[cmd_idx].redirect_out = tokens[tok_idx].value;
                tokens[tok_idx].value = NULL;
                cmds[cmd_idx].append = true;
            }
            else
            {
                fprintf(stderr, "shelly: expected filename after '>>'\n");
            }
            break;

        /* ── stderr redirect:  2> filename ── */
        case TOKEN_REDIRECT_ERR:
            tok_idx++;
            if (tok_idx < token_count && tokens[tok_idx].type == TOKEN_WORD)
            {
                cmds[cmd_idx].redirect_err = tokens[tok_idx].value;
                tokens[tok_idx].value = NULL;
            }
            else
            {
                fprintf(stderr, "shelly: expected filename after '2>'\n");
            }
            break;
        }

        tok_idx++;
    }

    /* NULL-terminate last command's args */
    cmds[cmd_idx].args[cmds[cmd_idx].arg_count] = NULL;
    *out_cmd_count = cmd_idx + 1;
    return cmds;
}

void free_commands(Command *cmds, int count)
{
    for (int i = 0; i < count; i++)
    {
        for (int j = 0; j < cmds[i].arg_count; j++)
            if (cmds[i].args[j]) free(cmds[i].args[j]);
        free(cmds[i].args);
        if (cmds[i].redirect_in)  free(cmds[i].redirect_in);
        if (cmds[i].redirect_out) free(cmds[i].redirect_out);
        if (cmds[i].redirect_err) free(cmds[i].redirect_err);
    }
    free(cmds);
}

/* ─────────────────────────────────────────────
 *  Legacy wrapper  (keeps old call-sites working)
 *  Returns a simple NULL-terminated char** for
 *  single commands with no pipes/redirects.
 * ───────────────────────────────────────────── */
char **input_parser(char *input)
{
    int token_count = 0;
    Token *tokens   = tokenize(input, &token_count);

    char **args = malloc(sizeof(char *) * MAX_TOKENS);
    if (!args) { perror("malloc"); exit(EXIT_FAILURE); }

    int j = 0;
    for (int i = 0; i < token_count; i++)
    {
        if (tokens[i].type == TOKEN_WORD && tokens[i].value)
            args[j++] = tokens[i].value;   /* ownership transferred */
        else
            if (tokens[i].value) free(tokens[i].value);
    }
    args[j] = NULL;

    free(tokens);    /* shells, not values – those are now in args[] */
    return args;
}