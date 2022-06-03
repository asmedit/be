#ifndef XT_DASM_H
#define XT_DASM_H

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "../editor.h"

void nasm_init();
void editor_render_dasm(struct editor* e, struct charbuf* b);
void editor_move_cursor_dasm(struct editor* e, int dir, int amount);
void editor_replace_byte_dasm(struct editor* e, char x);
void editor_insert_byte_dasm(struct editor* e, char x, bool after);
void editor_scroll_dasm(struct editor* e, int units);

#endif
