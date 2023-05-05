// Copyright (c) Namdak Tonpa
// PDP-11

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "../../editor.h"

struct Plain { const char* name; unsigned code; };
struct Immediate { const char* name; unsigned code; short upper; };

char* regs[] = { "r0","r1","r2","r3","r4","r5","sp","pc", 0 };

struct Plain direct[] = {
  { .name = "halt", .code = 0 },    { .name = "wait",  .code = 1 },
  { .name = "rti",  .code = 2 },    { .name = "bpt",   .code = 3 },
  { .name = "iot",  .code = 4 },    { .name = "reset", .code = 5 },
  { .name = "rtt",  .code = 6 },    { .name = "nop", .code = 240 },
  { .name = "clc", .code = 241 },   { .name = "clv", .code = 242 },
  { .name = "clz", .code = 244 },   { .name = "cln", .code = 250 },
  { .name = "sec", .code = 261 },   { .name = "sev", .code = 262 },
  { .name = "sez", .code = 264 },   { .name = "sen", .code = 270 },
  { .name = "scc", .code = 277 },   { .name = "ccc", .code = 257},
  { .name = "ret", .code = 207},    { .name = 0, .code = 0 } };

struct Plain one[] = {
  { .name = "jmp", .code = 1 },     { .name = "swab", .code = 3 },
  { .name = "clr", .code = 50 },    { .name = "clrb", .code = 1050 },
  { .name = "com", .code = 51 },    { .name = "comb", .code = 1051 },
  { .name = "inc", .code = 52 },    { .name = "incb", .code = 1052 },
  { .name = "dec", .code = 53 },    { .name = "decb", .code = 1053 },
  { .name = "neg", .code = 54 },    { .name = "negb", .code = 1054 },
  { .name = "adc", .code = 55 },    { .name = "adcb", .code = 1055 },
  { .name = "sbc", .code = 56 },    { .name = "sbcb", .code = 1056 },
  { .name = "tst", .code = 57 },    { .name = "tstb", .code = 1057 },
  { .name = "ror", .code = 60 },    { .name = "rorb", .code = 1060 },
  { .name = "rol", .code = 61 },    { .name = "rolb", .code = 1061 },
  { .name = "asr", .code = 62 },    { .name = "asrb", .code = 1062 },
  { .name = "asl", .code = 63 },    { .name = "aslb", .code = 1063 },
  { .name = "???", .code = 64 },    { .name = "mtps", .code = 1064 },
  { .name = "sxt", .code = 67 },    { .name = "mfps", .code = 1067 },
  { .name = 0, .code = 0 } };

struct Plain jmp[] = {
  { .name = "br",   .code = 04 },   { .name = "bne", .code = 10 },
  { .name = "beq",  .code = 14 },   { .name = "bge", .code = 20 },
  { .name = "blt",  .code = 24 },   { .name = "bgt", .code = 30 },
  { .name = "ble",  .code = 34 },   { .name = "bpl", .code = 1000 },
  { .name = "bmi",  .code = 1004 }, { .name = "bhi", .code = 1010 },
  { .name = "bvs",  .code = 1020 }, { .name = "bvc", .code = 1024 },
  { .name = "bhis", .code = 1030 }, { .name = "bcc", .code = 1030 },
  { .name = "blo",  .code = 1034 }, { .name = "bcs", .code = 1034 },
  { .name = "blos", .code = 1014 }, { .name = 0, .code = 0 } };

struct Immediate imm[] = {
  { .name = "emt", .code = 104000, .upper = 377 },
  { .name = "trap", .code = 104400, .upper = 377 },
  { .name = "mark", .code = 6400, .upper = 77 },
  { .name = 0, .code = 0, .upper = 0 } };

char* two[] = {
  "x", "mov",  "cmp",  "bit",  "bic",  "bis",  "add", "y",
  "z", "movb", "cmpb", "bitb", "bicb", "bisb", "sub", "w", 0 };

struct Plain xor[] = {
  { .name = "jsr", .code = 4000 },
  { .name = "xor", .code = 74000 },
  { .name = 0, .code = 0 } };

char* reg_addr[] = { "%s", "(%s)", "(%s)+", "@(%s)+", "-(%s)", "@-(%s)", "%X(%s)", "@%X(%s)", 0 };
char pdpout[1000];

uint16_t pdp11word(unsigned long int address) {
    uint16_t operation = (uint16_t)*((unsigned long int *)address);
    operation = (operation << 8) | (operation & 0xFF);
    return operation;
}

char * decodePDP11(unsigned long int address, char *outbuf, int *lendis)
{
    struct editor *e = editor();
    unsigned long int start = address, i;
    unsigned long int finish = address + 2;
    uint16_t operation = pdp11word(start);
    for (i = 0; i < 1000; i++) pdpout[i] = 0;

    for (i = 0; direct[i].name; i++) if (operation == direct[i].code) {
         sprintf(pdpout, "%s", direct[i].name);
         goto end;
    }

    for (i = 0; one[i].name; i++) if ((operation >> 6) == one[i].code) {
         uint8_t dst_reg = (operation >> 0) & 7;
         uint8_t dst_mod = (operation >> 3) & 7;
         sprintf(pdpout, "%s ", one[i].name);
         sprintf(pdpout+strlen(pdpout), reg_addr[dst_mod], regs[dst_reg]);
         goto end;
    }

    for (i = 0; jmp[i].name; i++) if ((operation >> 8) == (jmp[i].code >> 2)) {
         sprintf(pdpout, "%s 0x%x", jmp[i].name, ((operation & 0x80) ? (operation | ~0xFF) : (operation & 0xFF)) * 2 + 0 + 2);
         goto end;
    }

    for (i = 0; imm[i].name; i++) if ((operation & ~imm[i].upper) == imm[i].code) {
         sprintf(pdpout, "%s %u.", imm[i].name, operation & imm[i].upper);
         goto end;
    }

    for (i = 0; two[i]; i++) if ((operation >> 12) == i) {
         uint8_t dst_reg = (operation >> 0) & 7;
         uint8_t dst_mod = (operation >> 3) & 7;
         uint8_t src_reg = (operation >> 6) & 7;
         uint8_t src_mod = (operation >> 9) & 7;
         sprintf(pdpout, "%s ", two[i]);
         if (src_mod < 6) sprintf(pdpout+strlen(pdpout), reg_addr[src_mod], regs[src_reg]);
         else { sprintf(pdpout+strlen(pdpout), reg_addr[src_mod], pdp11word(finish), regs[src_reg]); finish += 2; }
         sprintf(pdpout+strlen(pdpout), ", ");
         if (dst_mod < 6) sprintf(pdpout+strlen(pdpout), reg_addr[dst_mod], regs[dst_reg]);
         else { sprintf(pdpout+strlen(pdpout), reg_addr[dst_mod], pdp11word(finish), regs[dst_reg]); finish += 2; }
         goto end;
    }

    for (i = 0; xor[i].name; i++) if ((operation & 777000) == xor[i].code) {
         uint8_t dst_reg = (operation >> 0) & 7;
         uint8_t dst_mod = (operation >> 3) & 7;
         sprintf(pdpout, "%s %s, ", xor[i].name, regs[(operation >> 6) & 7]);
         sprintf(pdpout+strlen(pdpout), reg_addr[dst_mod], regs[dst_reg]);
         goto end;
    }

    sprintf(pdpout, ".word 0x%x", operation);

end:
    memcpy(outbuf,pdpout,strlen(pdpout));
    outbuf[strlen(pdpout)] = '\0';
    *lendis = finish - start;
    return outbuf;
}

