#include "shelly.h"
#define LINE_MAX 1024

// Redraws the prompt + buffer, positions cursor at `pos`
static void redraw(const char *prompt, char *buf, int len, int pos) {
    // \r moves to start of line, \x1b[K clears to end
    printf("\r\x1b[K%s%s", prompt, buf);
    // move cursor back if not at end
    int back = len - pos;
    if (back > 0) printf("\x1b[%dD", back);
    fflush(stdout);
}

char *shell_readline(const char *prompt) {
    static char buf[LINE_MAX];
    int len = 0, pos = 0;

    term_raw_enable();
    printf("%s", prompt);
    fflush(stdout);

    while (1) {
        int c = term_read_char();
        if (c < 0) break;

        if (c == '\t') {
            // → call your completion function (Step 3)
            complete(buf, &len, &pos, prompt);
        }
        else if (c == '\r' || c == '\n') {
            printf("\r\n");
            break;
        }
        else if (c == 127 || c == '\b') {   // Backspace
            if (pos > 0) {
                memmove(&buf[pos-1], &buf[pos], len - pos);
                len--; pos--;
                buf[len] = '\0';
                redraw(prompt, buf, len, pos);
            }
        }
        else if (c == '\x1b') {             // Escape sequence (arrows)
            int c2 = term_read_char();
            int c3 = term_read_char();
            if (c2 == '[') {
                if (c3 == 'C' && pos < len) pos++;       // Right
                else if (c3 == 'D' && pos > 0) pos--;    // Left
                // Up/Down → history (future)
            }
            redraw(prompt, buf, len, pos);
        }

#ifdef _WIN32
        // Windows arrow keys come as 0x00/0xe0 prefix
        else if (c == 0x00 || c == 0xe0) {
            int c2 = term_read_char();
            if (c2 == 0x4d && pos < len) pos++;   // Right
            if (c2 == 0x4b && pos > 0)  pos--;    // Left
            redraw(prompt, buf, len, pos);
        }
#endif

        else if (c >= 32 && len < LINE_MAX - 1) {  // Printable char
            memmove(&buf[pos+1], &buf[pos], len - pos);
            buf[pos++] = (char)c;
            len++;
            buf[len] = '\0';
            redraw(prompt, buf, len, pos);
        }
    }

    term_raw_disable();
    buf[len] = '\0';
    return buf;
}