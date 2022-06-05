#ifndef XT_TERMINAL_H
#define XT_TERMINAL_H

#define XT_RELEASE_CODENAME "Fabrice"
#define XT_RELEASE_DATE "5.6.2022"
#define XT_VERSION "0.86.2"

#include <stdbool.h>
#include <termios.h>

enum key_codes {
    KEY_NULL      = 0,
    KEY_CTRL_D    = 0x04,
    KEY_CTRL_H    = 0x08,
    KEY_CTRL_Q    = 0x11, // DC1, to exit the program.
    KEY_CTRL_R    = 0x12, // DC2, to redo an action.
    KEY_CTRL_S    = 0x13, // DC3, to save the current buffer.
    KEY_CTRL_U    = 0x15,
    KEY_ESC       = 0x1b, // ESC, for things like keys up, down, left, right, delete, ...
    KEY_ENTER     = 0x0d,
    KEY_BACKSPACE = 0x7f,

    KEY_UP      = 1000, // [A
    KEY_DOWN,           // [B
    KEY_RIGHT,          // [C
    KEY_LEFT,           // [D
    KEY_DEL,            // . = 1b, [ = 5b, 3 = 33, ~ = 7e,
    KEY_HOME,           // [H
    KEY_END,            // [F
    KEY_PAGEUP,         // ??
    KEY_PAGEDOWN,       // ??
};

enum parse_errors {
    PARSE_SUCCESS,
    PARSE_INCOMPLETE_BACKSLASH,  // "...\"
    PARSE_INCOMPLETE_HEX,        // "...\x" or "...\xA"
    PARSE_INVALID_HEX,           // "...\xXY..." and X or Y not in [a-zA-Z0-9]
    PARSE_INVALID_ESCAPE,        // "...\a..." and a is not '\' or 'x'
};

void term_state_save();
void term_state_restore();
void enable_raw_mode();
void disable_raw_mode();
void clear_screen();
int  read_key();
int  hex2bin(const char* s);
void gotoxy(int rows, int cols);
bool get_window_size(int* rows, int* cols);
bool is_pos_num(const char* s);
bool is_hex(const char* s);
int hex2int(const char* s);
int clampi(int i, int min, int max);
int str2int(const char* s, int min, int max, int def);

#endif
