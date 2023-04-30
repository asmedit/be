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

#include "../arm/armadillo.h"

#include "../riscv/riscv-disas.h"

#include "../buffer.h"
#include "../editor.h"
#include "../terminal.h"

#include "../hex/hex.h"
#include "dasm.h"

#define LINES 140
#define DUMP  16
#define ADDR  8
#define CODE  64

#define ADDRWIN 16
#define DUMPWIN 32
#define CODEWIN 64

int  dump_win = DUMPWIN;
int  addr[LINES][ADDR];
int  codelen[LINES];
int  dumplen[LINES];
unsigned char dump[LINES][DUMP*20];
unsigned char code[LINES][CODE];

void nasm_init(struct editor* e) { nasm_ctype_init();  init_sync(); }

void setup_instruction(int i, struct editor* e, struct charbuf* b,
    uint64_t offset, uint8_t *data, int datalen, char *insn)
{
    dumplen[i] = datalen;
    memcpy(&dump[i],data,dumplen[i]);
    codelen[i] = strlen(insn) + 1;
    memcpy(&code[i],insn,codelen[i]);
}

void draw_instruction(int i, struct editor* e, struct charbuf* b,
    uint64_t offset, uint8_t *data, int datalen, char *insn)
{
    char hex[ 32 + 1];
    int  hexlen = 0;

    if (i + 1 == e->cursor_y) charbuf_appendf(b, "\x1b[1;97m\x1b[45m");
    else charbuf_appendf(b, "\x1b[0;93m\x1b[0;104m");
    charbuf_appendf(b, "%016x\x1b[0m ", offset);

    for (int j = 0; j < dumplen[i] && j < dump_win; j++)
        if (e->cursor_y - 1 == i && e->cursor_x - 1 == j)
        {
            charbuf_appendf(b, "\x1b[1;37m\x1b[43m");
            memset(hexstr()+1, '\0', 1);
            if (hexstr_idx() == 1 && e->mode == MODE_REPLACE)
                hexlen = snprintf(hex, sizeof(hex), "%02x", dump[i][j] & 0xF | hex2bin(hexstr()) & 0xF << 4);
	        charbuf_appendf(b, "%02X", (unsigned char)dump[i][j]);
            charbuf_appendf(b, "\x1b[4;94m\x1b[49m");
        } else {
            charbuf_appendf(b, "\x1b[4;94m\x1b[49m");
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
    int offset = e->offset_dasm;
    for (int i = 0; i < e->cursor_y - 1; i++) offset += dumplen[i];
    offset += e->cursor_x - 1;
    if (offset <= 0) return 0;
    if (offset >= e->content_length) return e->content_length - 1;
    return offset;
}

rv_isa bitness(struct editor* e)
{
    switch (e->seg_size) {
        case 32: rv32; break;
        case 64: rv64; break;
        case 128: rv128; break;
        default: rv64; break;
    }
}

void disassemble_screen(struct editor* e, struct charbuf* b)
{
    struct rv_inst *rvinst = NULL;
    struct ad_insn *insn = NULL;
    int offset = e->offset_dasm;
    char buffer[INSN_MAX * 2], *p, *ep, *q, outbuf[2048];
    unsigned int * opcode = NULL;
    uint32_t nextsync, synclen, initskip = 0L;
    int32_t lendis; bool autosync = false; iflag_t prefer; iflag_clear_all(&prefer);
    q = &e->contents[offset];
    p = &e->contents[0] + e->content_length;
    for (int i = 0; i < e->screen_rows - 2; i++) if (offset < e->content_length) {
        switch (e->arch) {
            case ARCH_INTEL: // Nasm
               lendis = disasm((uint8_t *)q, INSN_MAX, outbuf, sizeof(outbuf), e->seg_size, offset, autosync, &prefer);
               if (!lendis || lendis > (p - q) || ((nextsync || synclen) && (uint32_t)lendis > nextsync - offset))
                   lendis = eatbyte((uint8_t *) q, outbuf, sizeof(outbuf), e->seg_size); break;
            case ARCH_ARM: // Armadillo
               lendis = 4; opcode = q;
               ArmadilloDisassemble(*opcode, opcode, &insn);
               memcpy(outbuf,insn->decoded,strlen(insn->decoded));
               ArmadilloDone(&insn); break;
            case ARCH_RISCV: // SiFive
               inst_fetch((uint8_t *)q, &rvinst, &lendis);
               disasm_inst(outbuf, sizeof(outbuf), bitness(e), q, rvinst); break;
            default: break;
        }
        setup_instruction(i, e, b, offset, (uint8_t *) q, lendis, outbuf);
        q += lendis;
        offset += lendis;
    }
}


void editor_render_dasm(struct editor* e, struct charbuf* b)
{
    disassemble_screen(e, b);
    int offset = e->offset_dasm;
    for (int i = 0; i < e->screen_rows - 2; i++) {
        draw_instruction(i, e, b, offset, (uint8_t *)&dump[i][0], dumplen[i], &code[i][0]);
        offset += dumplen[i];
    }
}

void editor_move_cursor_dasm(struct editor* e, int dir, int amount)
{
    switch (dir) {
        case KEY_UP:    e->cursor_y-=amount; break; case KEY_DOWN:  e->cursor_y+=amount; break;
        case KEY_LEFT:  e->cursor_x-=amount; break; case KEY_RIGHT: e->cursor_x+=amount; break;
    }

    if (e->cursor_y <= 1) e->cursor_y = 1;
    if (e->cursor_y > e->screen_rows - 2) e->cursor_y = e->screen_rows - 2;
    if (e->cursor_x <= 1 && e->cursor_y <= 1 && e->line <= 0) { e->cursor_x = e->cursor_y = 1; return; }
    if (e->cursor_x < 1) {
        if (dir == KEY_LEFT) e->cursor_y--; else e->cursor_y++;
        if (e->cursor_y >= 1) e->cursor_x = dumplen[e->cursor_y - 1];
    } else if (e->cursor_x > dumplen[e->cursor_y - 1] && (dir == KEY_RIGHT || dir == KEY_LEFT)) {
        e->cursor_y++;
        e->cursor_x = 1;
    } else if (e->cursor_x > dumplen[e->cursor_y - 1] && (dir == KEY_DOWN || dir == KEY_UP)) {
        e->cursor_x = dumplen[e->cursor_y - 1];
    }
    if (e->cursor_y <= 1) e->cursor_y = 1;
    if (e->cursor_x <= 1) e->cursor_x = 1;
    if (e->cursor_y > e->screen_rows - 2) e->cursor_y = e->screen_rows - 2;
}

void editor_replace_byte_dasm(struct editor* e, char x) {
    hexstr_set(0,0); hexstr_set(1,0); hexstr_set(2,0);
    hexstr_idx_set(0);
    unsigned int offset = offset_at_cursor_dasm(e);
    unsigned char prev = e->contents[offset];
    e->contents[offset] = x;
    editor_refresh_screen(e);
    editor_move_cursor(e, KEY_RIGHT, 1);
    editor_statusmessage(e, STATUS_INFO, "Replaced byte at offset %09x with %02x", offset, (unsigned char) x);
    e->dirty = true;
}

void editor_insert_byte_dasm(struct editor* e, char x, bool after) {
}

void editor_scroll_dasm(struct editor* e, int units) {
}
