#include "shelly.h"

#ifdef _WIN32
#include <conio.h>

static DWORD orig_mode;

void term_raw_enable(void) {
    HANDLE h = GetStdHandle(STD_INPUT_HANDLE);
    GetConsoleMode(h, &orig_mode);
    // disable line input and echo
    SetConsoleMode(h, orig_mode
        & ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT));
}

void term_raw_disable(void) {
    SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), orig_mode);
}

int term_read_char(void) {
    return _getch();  // doesn't echo, returns immediately
}

#else
#include <termios.h>

static struct termios orig_termios;

void term_raw_enable(void) {
    tcgetattr(STDIN_FILENO, &orig_termios);
    struct termios raw = orig_termios;
    // disable canonical mode and echo
    raw.c_lflag &= ~(ICANON | ECHO | ISIG);
    raw.c_cc[VMIN]  = 1;  // read blocks until 1 char
    raw.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void term_raw_disable(void) {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

int term_read_char(void) {
    unsigned char c;
    if (read(STDIN_FILENO, &c, 1) <= 0) return -1;
    return c;
}
#endif