// Copyright (c) Namdak Tonpa
// PDP-11

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

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
  { .name = "sxt", .code = 67 },    { .name = "mtps", .code = 1064 },
  { .name = "mfps", .code = 1067 }, { .name = 0, .code = 0 } };

struct Plain jump[] = {
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

const char* two[] = {
  "", "mov", "cmp", "bit", "bic", "bis", "add", "", "",
  "movb", "cmpb", "bitb", "bicb", "bisb", "sub", 0 };

struct Plain xor[] = {
  { .name = "jsr", .code = 4000 },
  { .name = "xor", .code = 74000 },
  { .name = 0, .code = 0 } };

const char* fmt_mode[] = {
  "%s", "(%s)", "(%s)+", "@(%s)+", "-(%s)", "@-(%s)", "%u(%s)", "@%u(%s)",
  "pc", "(pc)", "#%u",   "@#%u",   "-(pc)", "@-(pc)", "%u",     "(%u)" };

const int mode[] = { 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, };

char * decodePDP11(unsigned long int address, char *outbuf, int *lendis)
{
    uint32_t operation = (uint32_t)*((unsigned long int *)address);
    memcpy(outbuf,"\0",1);
    *lendis = 2;
    return outbuf;
}
