#include <ctype.h>
#include <errno.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

#include "terminal.h"

static struct termios orig_termios;
char seq[4];

int hex2bin(const char* s) {
    int ret=0;
    for(int i = 0; i < 2; i++) {
        char c = *s++;
        int n=0;
        if( '0' <= c && c <= '9') n = c-'0';
        else if ('a' <= c && c <= 'f') n = 10 + c - 'a';
        else if ('A' <= c && c <= 'F') n = 10 + c - 'A';
        ret = n + ret*16;
    }
    return ret;
}

bool is_pos_num(const char* s) {
    for (const char* ptr = s; *ptr; ptr++) if (!isdigit(*ptr)) return false;
    return true;
}

bool is_hex(const char* s) {
    const char* ptr = s;
    while(*++ptr) if (!isxdigit(*ptr)) return false;
    return true;
}

int hex2int(const char* s) {
    char* endptr;
    intmax_t x = strtoimax(s, &endptr, 16);
    if (errno == ERANGE) return 0;
    return x;
}

inline int clampi(int i, int min, int max) {
    if (i < min) return min;
    if (i > max) return max;
    return i;
}

int str2int(const char* s, int min, int max, int def) {
    char* endptr;
    errno = 0;
    intmax_t x = strtoimax(s, &endptr, 10);
    if (errno  == ERANGE) return def;
    if (x < min || x > max) return def;
    return x;
}

int read_key() {
    char c;
    ssize_t nread;
    while ((nread = read(STDIN_FILENO, &c, 1)) == 0);
    if (nread == -1) return -1;
    if (c == KEY_ESC) {
        if (read(STDIN_FILENO, seq, 1) == 0) return KEY_ESC;
        if (read(STDIN_FILENO, seq + 1, 1) == 0) return KEY_ESC;
        if (seq[0] == '[') {
            if (seq[1] >= '0' && seq[1] <= '9') {
                if (read(STDIN_FILENO, seq + 2, 1) == 0) {
                    return KEY_ESC;
                }
                if (seq[2] == '~') {
                    switch (seq[1]) {
                        case '1': return KEY_HOME;
                        case '3': return KEY_END;
                        case '4': return KEY_DEL;
                        case '5': return KEY_PAGEUP;
                        case '6': return KEY_PAGEDOWN;
                        case '7': return KEY_HOME;
                        case '8': return KEY_END;
                    }
                }
            }
            switch (seq[1]) {
                case 'A': return KEY_UP;
                case 'B': return KEY_DOWN;
                case 'C': return KEY_RIGHT;
                case 'D': return KEY_LEFT;
                case 'H': return KEY_HOME;
                case 'F': return KEY_END;
            }
        } else if (seq[0] == 'O') {
            switch (seq[1]) {
                case 'H': return KEY_HOME;
                case 'F': return KEY_END;
            }
        }
    } else switch (c) {
        case KEY_BACKSPACE:
        case KEY_CTRL_H: return KEY_BACKSPACE;
    }
    return c;
}

bool get_window_size(int* rows, int* cols) {
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) != 0) {
        perror("Failed to query terminal size");
        exit(1);
    }

    *rows = ws.ws_row;
    *cols = ws.ws_col;
    return true;
}

void term_state_save() {
    (void) (write(STDOUT_FILENO, "\x1b[?1049h", 8) + 1);
}

void term_state_restore() {
    (void) (write(STDOUT_FILENO, "\x1b[?1049l", 8) + 1);
}

void enable_raw_mode() {
    if (!isatty(STDIN_FILENO)) {
        perror("Input is not a TTY");
        exit(1);
    }

    tcgetattr(STDIN_FILENO, &orig_termios);

    struct termios raw = orig_termios;
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag |= (CS8);
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) != 0) {
        perror("Unable to set terminal to raw mode");
        exit(1);
    }
}

void disable_raw_mode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
    (void) (write(STDOUT_FILENO, "\x1b[?25h", 6) + 1);
}

void clear_screen() {
    char stuff[80];
    int bw = snprintf(stuff, 80, "\x1b[0m\x1b[H\x1b[2J");
    if (write(STDOUT_FILENO, stuff, bw) == -1) {
        perror("Unable to clear screen");
    }
}
