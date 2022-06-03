#include <assert.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>

#include "../x86/compiler.h"
#include "../x86/nctype.h"
#include "../x86/insns.h"
#include "../x86/nasm.h"
#include "../x86/nasmlib.h"
#include "../x86/error.h"
#include "../x86/ver.h"
#include "../x86/sync.h"
#include "../x86/disasm.h"

#include "../buffer.h"
#include "../editor.h"
#include "../terminal.h"

#include "../hex/hex.h"
#include "dasm.h"

// 120-column Layout: 16-32-64

// 0000000000003F97 B804000002                      mov eax,0x2000004
// 0000000000003F9C BF010000F2                      mov edi,0xf2000001
// 0000000000003FA1 3648818424756341278563412F23412 lwpins rax,[fs:eax+ebx+0x12345678],0x12345678
// 0000000000003FBB F2F0364000000000
// 0000000000003FBB F2F0364881842475634127856340000 xacquire lock add [ss:rsp+0x12345678],0x12345678
// 0000000000003FB8 C3                              ret

#define LINES 40
#define DUMP  16
#define ADDR  8
#define CODE  64

#define ADDRWIN 16
#define DUMPWIN 32
#define CODEWIN 64

char buffer[INSN_MAX * 2], *p, *ep, *q;
char outbuf[256];
uint32_t nextsync, synclen, initskip = 0L;
int32_t lendis, lenins;
bool autosync = false;
int bits = 16, b;
bool eof = false;
iflag_t prefer;
bool rn_error;
unsigned long offset;
char dump[LINES][DUMP];
char code[LINES][CODE];
char addr[LINES][ADDR];

void nasm_init() {
    nasm_ctype_init();
    iflag_clear_all(&prefer);
    offset = 0;
    init_sync();
}

void output_inst(int i, struct editor* e, struct charbuf* b, uint64_t offset, uint8_t *data, int datalen, char *insn)
{
    int bytes; int BPL = 8;

    if (i + 1 == e->cursor_y) charbuf_appendf(b, "\x1b[1;97m\x1b[45m");
    else charbuf_appendf(b, "\x1b[0;93m\x1b[0;104m");
    charbuf_appendf(b, "%016x\x1b[0m ", offset);

    bytes = 0;
    while (datalen > 0 && bytes < BPL*2) { charbuf_appendf(b, "%02X", *data++); bytes++; datalen--; }
    charbuf_appendf(b, "\x1b[0m ");
    charbuf_appendf(b, "\x1b[0;93m\x1b[0;104m");
    for (int i=0; i < 2 * (BPL-bytes); i++) charbuf_appendf(b, " ");

    charbuf_appendf(b, "\x1b[0m ");
    if (i + 1 == e->cursor_y) charbuf_appendf(b, "\x1b[1;97m\x1b[45m");
    else charbuf_appendf(b, "\x1b[0;93m\x1b[0;104m");

    lenins = strlen(insn);
    charbuf_appendf(b, "%s", insn);
    for (int i=0; i < 46 - lenins; i++) charbuf_appendf(b, " ");
    charbuf_appendf(b, "\r\n");

}

void editor_render_dasm(struct editor* e, struct charbuf* b) {
    bits = e->seg_size;
    p = q = b->contents;
    nextsync = next_sync(offset, &synclen);
    p += e->content_length;
    offset = editor_offset_at_cursor(e);

    for (int i=0;i<30;i++) {

        lendis = disasm((uint8_t *)q, INSN_MAX, outbuf, sizeof(outbuf), bits, offset, autosync, &prefer);
        if (!lendis || lendis > (p - q) || ((nextsync || synclen) && (uint32_t)lendis > nextsync - offset))
            lendis = eatbyte((uint8_t *) q, outbuf, sizeof(outbuf), bits);

        output_inst(i, e, b, offset, (uint8_t *) q, lendis, outbuf);

        q += lendis;
        offset += lendis;

    }
}


void editor_move_cursor_dasm(struct editor* e, int dir, int amount) {
}

void editor_replace_byte_dasm(struct editor* e, char x) {
}

void editor_insert_byte_dasm(struct editor* e, char x, bool after) {
}

void editor_scroll_dasm(struct editor* e, int units) {
}
