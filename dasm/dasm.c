#include <assert.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>

#include "../arch/x86/compiler.h"
#include "../arch/x86/nctype.h"
#include "../arch/x86/insns.h"
#include "../arch/x86/nasm.h"
#include "../arch/x86/nasmlib.h"
#include "../arch/x86/error.h"
#include "../arch/x86/ver.h"
#include "../arch/x86/sync.h"
#include "../arch/x86/disasm.h"
#include "../arch/arm/armadillo.h"
#include "../arch/riscv/riscv-disas.h"
#include "../arch/ppc/ppc_disasm.h"
#include "../arch/sh4/sh4dis.h"

#include "../term/buffer.h"
#include "../term/terminal.h"
#include "../hex/hex.h"
#include "dasm.h"
#include "../editor.h"

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
        case 32:  return rv32;
        case 64:  return rv64;
        case 128: return rv128;
        default:  return rv64;
    }
}

#define SH4ASM_TXT_LEN 228
static char sh4asm_disas[SH4ASM_TXT_LEN];
unsigned sh4asm_disas_len;
static void clear_asm(void) { sh4asm_disas_len = 0; }
static void neo_asm_emit(char ch) {
    if (sh4asm_disas_len >= SH4ASM_TXT_LEN)
        errx(1, "sh4asm disassembler buffer overflow");
    sh4asm_disas[sh4asm_disas_len++] = ch;
}

void disassemble_screen(struct editor* e, struct charbuf* b)
{
    struct DisasmPara_PPC dp;
    char ppc_opcode[640];
    char ppc_operands[2560];
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
               memcpy(outbuf,insn->decoded,strlen(insn->decoded)+1);
               ArmadilloDone(&insn); break;
            case ARCH_RISCV: // SiFive
               inst_fetch((uint8_t *)q, &rvinst, &lendis);
               disasm_inst(outbuf, sizeof(outbuf), bitness(e), q, rvinst); break;
            case ARCH_PPC: // PowerPC
               dp.opcode = ppc_opcode;
               dp.operands = ppc_operands;
               dp.iaddr = (unsigned int *)q;
               dp.instr = (unsigned int *)q;
               PPC_Disassemble(&dp);
               memcpy(outbuf,ppc_opcode,strlen(ppc_opcode));
               memcpy(outbuf+strlen(ppc_opcode)," ",1);
               memcpy(outbuf+strlen(ppc_opcode)+1,ppc_operands,strlen(ppc_operands));
               memcpy(outbuf+strlen(ppc_opcode)+1+strlen(ppc_operands),"\0",1);
               lendis = sizeof(ppc_word);
               break;
            case ARCH_SH4: // SuperH-4
               uint16_t inst_bin = (uint16_t)*q;
               memset(sh4_buf, 0, sizeof(sh4_buf));
               clear_asm();
               sh4asm_disas_inst(inst_bin, neo_asm_emit, 0);
               memcpy(outbuf,sh4asm_disas,sh4asm_disas_len);
               memcpy(outbuf+sh4asm_disas_len,"\0",1);
               lendis = 2;
               break;
            case ARCH_M68K: // M68K
               disasm68k((unsigned long int)q,(unsigned long int)q+10,outbuf,&lendis);
               break;
            case ARCH_MIPS: // M68K
               uint32_t inst = (uint32_t)*((unsigned long int *)q);
               decodeMIPS(inst,(unsigned long int)q,outbuf);
               lendis = 4;
               break;
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
