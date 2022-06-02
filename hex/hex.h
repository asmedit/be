#ifndef XT_HEX_H
#define XT_HEX_H

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

void editor_cursor_at_offset(struct editor* e, int offset, int* x, int* y);
int  editor_offset_at_cursor(struct editor* e);
void editor_scroll_to_offset(struct editor* e, unsigned int offset);

void editor_move_cursor_hex(struct editor* e, int dir, int amount);
void editor_scroll_hex(struct editor* e, int units);

void editor_replace_byte_hex(struct editor* e, char x);
void editor_insert_byte_hex(struct editor* e, char x, bool after);
void editor_insert_byte_at_offset(struct editor* e, unsigned int offset, char x, bool after);

void editor_render_ascii(struct editor* e, int rownum, unsigned int start_offset, struct charbuf* b);
void editor_render_hex(struct editor* e, struct charbuf* b);

#endif
