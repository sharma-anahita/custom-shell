#include "shelly.h"
#include <dirent.h>

// Find the word currently being typed (last whitespace-delimited token)
static char *current_word(char *buf, int pos) {
    int start = pos;
    while (start > 0 && buf[start-1] != ' ') start--;
    return &buf[start];
}

// Collect matches into a char** list
static char **get_completions(char *prefix, int *count) {
    char **matches = malloc(sizeof(char*) * 256);
    *count = 0;

    // --- 1. Try PATH completion (first token = command) ---
    // --- 2. Try file/dir completion (other tokens) --------
    // For simplicity, always do file completion:

    char dir_part[MAX_PATH_LENGTH] = ".";
    char *file_part = prefix;

    // split prefix into dir + file if it contains a slash
    char *last_slash = strrchr(prefix, DIR_SEP);
    if (last_slash) {
        size_t dlen = last_slash - prefix;
        memcpy(dir_part, prefix, dlen);
        dir_part[dlen] = '\0';
        file_part = last_slash + 1;
    }

    DIR *d = opendir(dir_part);
    if (!d) return matches;

    struct dirent *entry;
    size_t prefix_len = my_strLen(file_part);

    while ((entry = readdir(d)) != NULL && *count < 255) {
        if (entry->d_name[0] == '.') continue;  // skip hidden
        if (my_strncmp(entry->d_name, file_part, prefix_len, true) == 0) {
            // rebuild full match: dir_part + DIR_SEP + name
            char full[MAX_PATH_LENGTH];
            if (last_slash)
                snprintf(full, sizeof(full), "%.*s%c%s",
                         (int)(last_slash - prefix + 1), prefix,
                         DIR_SEP, entry->d_name);
            else
                snprintf(full, sizeof(full), "%s", entry->d_name);

            matches[(*count)++] = my_strdup(full);
        }
    }
    closedir(d);
    return matches;
}

void complete(char *buf, int *len, int *pos, const char *prompt) {
    char *word  = current_word(buf, *pos);
    int   wlen  = my_strLen(word);
    int   count = 0;
    char **matches = get_completions(word, &count);

    if (count == 0) {
        // no match: ring the bell
        printf("\a");
        fflush(stdout);
    }
    else if (count == 1) {
        // single match: complete it
        char *m   = matches[0];
        int   mlen = my_strLen(m);
        int   add  = mlen - wlen;
        // insert `add` chars into buf at *pos
        memmove(&buf[*pos + add], &buf[*pos], *len - *pos);
        memcpy(&buf[*pos - wlen + wlen], m + wlen, add);
        *pos += add;
        *len += add;
        buf[*len] = '\0';
        // redraw
        printf("\r\x1b[K%s%s", prompt, buf);
        int back = *len - *pos;
        if (back > 0) printf("\x1b[%dD", back);
        fflush(stdout);
    }
    else {
        // multiple matches: print them below, redraw prompt
        printf("\r\n");
        for (int i = 0; i < count; i++) {
            printf("  %s\n", matches[i]);
        }
        printf("%s%s", prompt, buf);
        int back = *len - *pos;
        if (back > 0) printf("\x1b[%dD", back);
        fflush(stdout);
    }

    for (int i = 0; i < count; i++) free(matches[i]);
    free(matches);
}