#ifndef XT_EDITOR_H
#define XT_EDITOR_H

#include <unistd.h>
#include <sys/ioctl.h>
#include <stdbool.h>
#include "buffer.h"

enum editor_view {
    VIEW_HEX = 1,
    VIEW_ASM = 2,
};

enum dasm_arch {
    ARCH_INTEL = 1,
    ARCH_ARM = 2,
    ARCH_RISCV = 3,
    ARCH_PPC = 4,
    ARCH_SH4 = 5,
    ARCH_M68K = 6,
};

enum editor_mode {
    MODE_APPEND        = 1 << 0,
    MODE_APPEND_ASCII  = 1 << 1,
    MODE_REPLACE_ASCII = 1 << 2,
    MODE_NORMAL        = 1 << 3,
    MODE_INSERT        = 1 << 4,
    MODE_INSERT_ASCII  = 1 << 5,
    MODE_REPLACE       = 1 << 6,
    MODE_COMMAND       = 1 << 7,
    MODE_SEARCH        = 1 << 8,
};

enum search_direction {
    SEARCH_FORWARD,
    SEARCH_BACKWARD,
};

enum status_severity {
    STATUS_INFO,
    STATUS_WARNING,
    STATUS_ERROR,
};

#define INPUT_BUF_SIZE 80

struct editor {
    int octets_per_line;
    int grouping;
    int line;
    int cursor_x;
    int cursor_y;
    int hex_x;
    int hex_y;
    int screen_rows;
    int screen_cols;
    enum editor_mode mode;
    bool dirty;
    char* filename;
    char* contents;
    unsigned long offset_dasm;
    unsigned long offset_hex;
    enum editor_view view;
    enum dasm_arch arch;
    unsigned int content_length;
    enum status_severity status_severity;
    char status_message[120];
    char inputbuffer[INPUT_BUF_SIZE];
    int inputbuffer_index;
    char searchstr[INPUT_BUF_SIZE];
    int seg_size;
};

int  hexstr_idx_inc();
int  hexstr_idx_set(int value);
int  hexstr_idx();
char *hexstr();
char hexstr_set(int pos, int value);
char hexstr_get(int pos);

struct editor* editor_init();
void editor_free(struct editor* e);
void editor_openfile(struct editor* e, const char* filename);
void editor_refresh_screen(struct editor* e);
void editor_setmode(struct editor *e, enum editor_mode mode);
void editor_setview(struct editor *e, enum editor_view view);
int editor_statusmessage(struct editor* e, enum status_severity s, const char* fmt, ...);
void editor_writefile(struct editor* e);

void editor_move_cursor(struct editor* e, int dir, int amount);
void editor_process_keypress(struct editor* e);
void editor_scroll(struct editor* e, int units);

#endif
