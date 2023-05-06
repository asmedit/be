// Copyright (c) Namdak Tonpa
// PDP-11

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "../../editor.h"

struct Plain { const char* name; unsigned code; };
struct Immediate { const char* name; unsigned code; short bits; int mask; int arg; };

char *regs[] = { "r0", "r1", "r2", "r3", "r4", "r5", "sp", "pc", 0 };
char *fpus[] = { "ac0", "ac1", "ac2", "ac3", "ac4", "ac5", "ac6", "ac7", 0 };

struct Plain direct[] = {
  { .name = "halt", .code = 0 },     { .name = "wait",  .code = 1 },
  { .name = "rti",  .code = 2 },     { .name = "bpt",   .code = 3 },
  { .name = "iot",  .code = 4 },     { .name = "reset", .code = 5 },
  { .name = "rtt",  .code = 6 },     { .name = "ret",   .code = 0207 },
  { .name = "nop",  .code = 0240 },  { .name = "clc",   .code = 0241 },
  { .name = "clv",  .code = 0242 },  { .name = "clz",   .code = 0244 },
  { .name = "cln",  .code = 0250 },  { .name = "ccc",   .code = 0257 },
  { .name = "sec",  .code = 0261 },  { .name = "sev",   .code = 0262 },
  { .name = "sez",  .code = 0264 },  { .name = "sen",   .code = 0270 },
  { .name = "scc",  .code = 0277 },

  { .name = "cfcc", .code = 0170000 },
  { .name = "setf", .code = 0170001 },
  { .name = "seti", .code = 0170002 },
  { .name = "setd", .code = 0170011 },
  { .name = "setl", .code = 0170012 },
  { .name = 0, .code = 0 } };

struct Plain one[] = {
  { .name = "jmp",  .code = 1 },     { .name = "rts",   .code = 2 },
  { .name = "swab", .code = 3 },
  { .name = "clr",  .code = 050 },   { .name = "clrb",  .code = 01050 },
  { .name = "com",  .code = 051 },   { .name = "comb",  .code = 01051 },
  { .name = "inc",  .code = 052 },   { .name = "incb",  .code = 01052 },
  { .name = "dec",  .code = 053 },   { .name = "decb",  .code = 01053 },
  { .name = "neg",  .code = 054 },   { .name = "negb",  .code = 01054 },
  { .name = "adc",  .code = 055 },   { .name = "adcb",  .code = 01055 },
  { .name = "sbc",  .code = 056 },   { .name = "sbcb",  .code = 01056 },
  { .name = "tst",  .code = 057 },   { .name = "tstb",  .code = 01057 },
  { .name = "ror",  .code = 060 },   { .name = "rorb",  .code = 01060 },
  { .name = "rol",  .code = 061 },   { .name = "rolb",  .code = 01061 },
  { .name = "asr",  .code = 062 },   { .name = "asrb",  .code = 01062 },
  { .name = "asl",  .code = 063 },   { .name = "aslb",  .code = 01063 },
                                     { .name = "mtps",  .code = 01064 },
  { .name = "mfpi", .code = 065 },   { .name = "mfpd",  .code = 01065 },
  { .name = "mtpi", .code = 066 },   { .name = "mtpd",  .code = 01066 },
  { .name = "sxt",  .code = 067 },   { .name = "mfps",  .code = 01067 },
  { .name = "csm",  .code = 070 }, // PDP-11/44
  { .name = 0, .code = 0 } };

struct Plain jmp[] = {
  { .name = "br",   .code = 001 },   { .name = "bne", .code = 002 },
  { .name = "beq",  .code = 003 },   { .name = "bge", .code = 004 },
  { .name = "blt",  .code = 005 },   { .name = "bgt", .code = 006 },
  { .name = "ble",  .code = 007 },   { .name = "bpl", .code = 0200 },
  { .name = "bmi",  .code = 0201 },  { .name = "bhi", .code = 0202 },
  { .name = "blos", .code = 0203 },  { .name = "bvc", .code = 0204 },
  { .name = "bvs",  .code = 0205 },  { .name = "bcc", .code = 0206 },
  { .name = "bcs",  .code = 0207 },  { .name = 0, .code = 0 } };

struct Immediate imm[] = {
  { .name = "emt",  .code = 0210, .bits = 8, .mask = 0xFF, .arg = 0 },
  { .name = "trap", .code = 0211, .bits = 8, .mask = 0xFF, .arg = 0 },
  { .name = "mark", .code = 064,  .bits = 6, .mask = 0x3F, .arg = 1 },
  { .name = 0, .code = 0, .bits = 0, .mask = 0, .arg = 0 } };

struct Plain one3[] = {
  { .name = "rts",  .code = 020 },
  { .name = "fadd", .code = 07500 },
  { .name = "fsub", .code = 07501 },
  { .name = "fmul", .code = 07502 },
  { .name = "fdiv", .code = 07503 },
  { .name = 0, .code = 0 }
};

struct Plain sob[] = {
  { .name = "sob",  .code = 077 },
  { .name = 0, .code = 0 }
};

struct Plain xor[] = {
  { .name = "jsr",  .code = 04  },
  { .name = "mul",  .code = 070 },
  { .name = "div",  .code = 071 },
  { .name = "ash",  .code = 072 },
  { .name = "ashc", .code = 073 },
  { .name = "xor",  .code = 074 },
  { .name = 0, .code = 0 } };

struct Plain fpu1[] = {
  { .name = "ldfps",  .code = 01701 },
  { .name = "stfps",  .code = 01702 },
  { .name = "stst",   .code = 01703 },
  { .name = "clrf",   .code = 01704 },
  { .name = "tstf",   .code = 01705 },
  { .name = "absf",   .code = 01706 },
  { .name = "negf",   .code = 01707 },
  { .name = 0, .code = 0 } };

struct Plain fpu2[] = {
  { .name = "mulf",   .code = 0xF2 },
  { .name = "modf",   .code = 0xF3 },
  { .name = "addf",   .code = 0xF4 },
  { .name = "ldf",    .code = 0xF5 },
  { .name = "subf",   .code = 0xF6 },
  { .name = "cmpf",   .code = 0xF7 },
  { .name = "divf",   .code = 0xF9 },
  { .name = "stexp",  .code = 0xFA },
  { .name = "stcfi",  .code = 0xFB },
  { .name = "stcfd",  .code = 0xFC },
  { .name = "ldexp",  .code = 0xFD },
  { .name = "ldcif",  .code = 0xFE },
  { .name = "ldcdf",  .code = 0xFF },
  { .name = 0, .code = 0 } };

char* two[] = {
  0, "mov",  "cmp",  "bit",  "bic",  "bis",  "add", 0,
  0, "movb", "cmpb", "bitb", "bicb", "bisb", "sub", 0, 0 };

char* reg_addr[] = { "%s", "(%s)", "(%s)+", "@(%s)+", "-(%s)", "@-(%s)", "0x%X(%s)", "@0x%X(%s)", 0 };
char pdpout[1000];

uint16_t pdp11word(unsigned long int address) {
    uint16_t operation = (uint16_t)*((unsigned long int *)address);
    operation = (operation << 8) | (operation >> 8);
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

    for (i = 0; one3[i].name; i++) if ((operation >> 3) == one3[i].code) {
         uint8_t dst_reg = (operation >> 0) & 7;
         uint8_t dst_mod = (operation >> 3) & 7;
         sprintf(pdpout, "%s ", one3[i].name);
         sprintf(pdpout+strlen(pdpout), reg_addr[dst_mod], regs[dst_reg]);
         goto end;
    }

    for (i = 0; one[i].name; i++) if ((operation >> 6) == one[i].code) {
         uint8_t dst_reg = (operation >> 0) & 7;
         uint8_t dst_mod = (operation >> 3) & 7;
         sprintf(pdpout, "%s ", one[i].name);
         if (dst_mod < 6) sprintf(pdpout+strlen(pdpout), reg_addr[dst_mod], regs[dst_reg]);
         else { sprintf(pdpout+strlen(pdpout), reg_addr[dst_mod], pdp11word(finish), regs[dst_reg]); finish += 2; }
         goto end;
    }

    for (i = 0; fpu1[i].name; i++) if ((operation >> 6) == fpu1[i].code) {
         uint8_t dst_reg = (operation >> 0) & 7;
         uint8_t dst_mod = (operation >> 3) & 7;
         sprintf(pdpout, "%s ", fpu1[i].name);
         if (dst_mod < 6) sprintf(pdpout+strlen(pdpout), reg_addr[dst_mod], fpus[dst_reg]);
         else { sprintf(pdpout+strlen(pdpout), reg_addr[dst_mod], pdp11word(finish), fpus[dst_reg]); finish += 2; }
         goto end;
    }

    for (i = 0; fpu2[i].name; i++) if ((operation >> 8) == fpu2[i].code) {
         uint8_t dst_reg = (operation >> 0) & 7;
         uint8_t dst_mod = (operation >> 3) & 7;
         uint8_t ac      = (operation >> 6) & 3;
         sprintf(pdpout, "%s %s, ", fpu2[i].name, fpus[ac]);
         if (dst_mod < 6) sprintf(pdpout+strlen(pdpout), reg_addr[dst_mod], fpus[dst_reg]);
         else { sprintf(pdpout+strlen(pdpout), reg_addr[dst_mod], pdp11word(finish), fpus[dst_reg]); finish += 2; }
         goto end;
    }

    for (i = 0; jmp[i].name; i++) if ((operation >> 8) == jmp[i].code) {
         sprintf(pdpout, "%s 0x%x", jmp[i].name, (operation & 0xFF) * 2 + 2);
         goto end;
    }

    for (i = 0; sob[i].name; i++) if ((operation >> 9) == sob[i].code) {
         uint8_t reg = (operation >> 6) & 7;
         uint8_t offset = operation & 0x3F;
         sprintf(pdpout, "%s %s, 0x%x", sob[i].name, regs[reg], offset);
         goto end;
    }

    for (i = 0; xor[i].name; i++) if ((operation >> 9) == xor[i].code) {
         uint8_t dst_reg = (operation >> 0) & 7;
         uint8_t dst_mod = (operation >> 3) & 7;
         sprintf(pdpout, "%s %s, ", xor[i].name, regs[(operation >> 6) & 7]);
         if (dst_mod < 6) sprintf(pdpout+strlen(pdpout), reg_addr[dst_mod], regs[dst_reg]);
         else { sprintf(pdpout+strlen(pdpout), reg_addr[dst_mod], pdp11word(finish), regs[dst_reg]); finish += 2; }
         goto end;
    }

    for (i = 0; imm[i].name; i++) if ((operation >> imm[i].bits) == imm[i].code) {
         sprintf(pdpout, "%s %u.", imm[i].name, operation & imm[i].mask);
         goto end;
    }

    for (i = 0; i < 16; i++) if ((operation >> 12) == i && two[i]) {
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

    sprintf(pdpout, ".word 0x%04X", (unsigned int)operation);

end:
    memcpy(outbuf,pdpout,strlen(pdpout));
    outbuf[strlen(pdpout)] = '\0';
    *lendis = finish - start;
    return outbuf;
}

