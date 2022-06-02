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

// 120-column Layout: 16-32-64

// 0000000000003F97 B804000002                      mov eax,0x2000004
// 0000000000003F9C BF010000F2                      mov edi,0xf2000001
// 0000000000003FA1 3648818424756341278563412F23412 lwpins rax,[fs:eax+ebx+0x12345678],0x12345678
// 0000000000003FBB F2F0364000000000
// 0000000000003FBB F2F0364881842475634127856340000 xacquire lock add [ss:rsp+0x12345678],0x12345678
// 0000000000003FB8 C3                              ret

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
