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

void editor_process_keypress_dasm(struct editor* e);
void editor_render_dasm(struct editor* e, struct charbuf* b);

#endif
