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

#define LINES 140
#define DUMP  16
#define ADDR  8
#define CODE  64

#define ADDRWIN 16
#define DUMPWIN 32
#define CODEWIN 64

int dump_win = DUMPWIN;
char buffer[INSN_MAX * 2], *p, *ep, *q;
char outbuf[256];
uint32_t nextsync, synclen, initskip = 0L;
int32_t lendis, lenins;
bool autosync = false;
bool eof = false;
int bits;
iflag_t prefer;
unsigned long offset;
int addr[LINES][ADDR];
char dump[LINES][DUMP*20];
int dumplen[LINES];
char code[LINES][CODE];
int codelen[LINES];

void nasm_init(struct editor* e) { nasm_ctype_init(); iflag_clear_all(&prefer); init_sync(); }

void setup_inst(int i, struct editor* e, struct charbuf* b, uint64_t offset, uint8_t *data, int datalen, char *insn)
{
    // setup

    dumplen[i] = datalen;
    memcpy(&dump[i],data,dumplen[i]);
    lenins = strlen(insn);
    codelen[i] = lenins;
    memcpy(&code[i],insn,lenins+1);
}

void output_inst(int i, struct editor* e, struct charbuf* b, uint64_t offset, uint8_t *data, int datalen, char *insn)
{
    // draw

    char hex[ 32 + 1];
    int  hexlen = 0;
    if (i + 1 == e->cursor_y) charbuf_appendf(b, "\x1b[1;97m\x1b[45m");
    else charbuf_appendf(b, "\x1b[0;93m\x1b[0;104m");
    charbuf_appendf(b, "%016x\x1b[0m ", offset);
    for (int j = 0; j < dumplen[i] && j < dump_win; j++) //charbuf_appendf(b, "%02X", dump[i][j]);
	if (e->cursor_y == i + 1 && e->cursor_x == j + 1)
        {
            charbuf_appendf(b, "\x1b[1;37m\x1b[43m");
            memset(hexstr()+1, '\0', 1);
            if (hexstr_idx() == 1 && e->mode == MODE_REPLACE)
                hexlen = snprintf(hex, sizeof(hex), "%02x",
                dump[i][j] & 0xF | hex2bin(hexstr()) & 0xF << 4);
	    charbuf_appendf(b, "%02X", dump[i][j]);
        } else {
            charbuf_appendf(b, "\x1b[48m\x1b[4;94m");
	    charbuf_appendf(b, "%02X", dump[i][j]);
        }

    charbuf_appendf(b, "\x1b[0m ");
    charbuf_appendf(b, "\x1b[0;93m\x1b[0;104m");
    for (int j = 0; j < dump_win - 2 * dumplen[i]; j++) charbuf_appendf(b, " ");
    charbuf_appendf(b, "\x1b[0m ");
    if (i + 1 == e->cursor_y) charbuf_appendf(b, "\x1b[1;97m\x1b[45m");
    else charbuf_appendf(b, "\x1b[0;93m\x1b[0;104m");
    charbuf_appendf(b, "%s", &code[i]);
    for (int j = 0; j < 62 - strlen(code[i]); j++) charbuf_appendf(b, " ");
    charbuf_appendf(b, "\r\n");

}

int offset_at_cursor_dasm(struct editor* e) {
    offset = e->offset_dasm;
    for (int i = 0; i < e->cursor_y - 1; i++) offset += dumplen[i];
    offset += e->cursor_x - 1;
    if (offset <= 0) return 0;
    if (offset >= e->content_length) return e->content_length - 1;
    return offset;
}

void editor_render_dasm(struct editor* e, struct charbuf* b) {
    bits = e->seg_size;
    offset = e->offset_dasm;
    p = q = &e->contents[offset];
    p = &e->contents[0] + e->content_length;

    for (int i = 0; i < e->screen_rows - 2; i++) if (offset < e->content_length)
    {
        lendis = disasm((uint8_t *)q, INSN_MAX, outbuf, sizeof(outbuf), bits, offset, autosync, &prefer);
        if (!lendis || lendis > (p - q) || ((nextsync || synclen) && (uint32_t)lendis > nextsync - offset))
            lendis = eatbyte((uint8_t *) q, outbuf, sizeof(outbuf), bits);
        setup_inst(i, e, b, offset, (uint8_t *) q, lendis, outbuf);
        q += lendis;
        offset += lendis;
    }

    for (int i = 0; i < e->screen_rows - 2; i++)
        output_inst(i, e, b, offset += dumplen[i], (uint8_t *)&dump[i][0], dumplen[i], &code[i][0]);

}

void editor_move_cursor_dasm(struct editor* e, int dir, int amount) {
    switch (dir) {
        case KEY_UP:    e->cursor_y-=amount; break;
        case KEY_DOWN:  e->cursor_y+=amount; break;
        case KEY_LEFT:  e->cursor_x-=amount; break;
        case KEY_RIGHT: e->cursor_x+=amount; break;
    }

    if (e->cursor_x <= 1 && e->cursor_y <= 1 && e->line <= 0) {
        e->cursor_x = 1;
        e->cursor_y = 1;
        return;
    }

    if (e->cursor_x < 1) {
        if (e->cursor_y >= 1) {
            e->cursor_y--;
            e->cursor_x = dumplen[e->cursor_y - 1];
        }
    } else if (e->cursor_x > dumplen[e->cursor_y - 1]) {
        e->cursor_y++;
        e->cursor_x = 1;
    }

    if (e->cursor_y <= 1 && e->line <= 0) {
        e->cursor_y = 1;
    }
/*
    if (e->cursor_y > e->screen_rows - 2) {
        e->cursor_y = e->screen_rows - 2;
        editor_scroll(e, 1);
    } else if (e->cursor_y < 1 && e->line > 0) {
        e->cursor_y = 1;
        editor_scroll(e, -1);
    }
*/
}

void editor_replace_byte_dasm(struct editor* e, char x) {
}

void editor_insert_byte_dasm(struct editor* e, char x, bool after) {
}

void editor_scroll_dasm(struct editor* e, int units) {
}
