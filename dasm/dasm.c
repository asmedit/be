#include <assert.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>

#include "dasm.h"

#include "../buffer.h"
#include "../editor.h"
#include "../terminal.h"

// 
void editor_render_dasm(struct editor* e, struct charbuf* b) {
}

void editor_move_cursor_dasm(struct editor* e, int dir, int amount) {
}

void editor_replace_byte_dasm(struct editor* e, char x) {
}

void editor_insert_byte_dasm(struct editor* e, char x, bool after) {
}

void editor_scroll_dasm(struct editor* e, int units) {
}


