// Copyright (c) Namdak Tonpa
// nVidia G80 SM89

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "../../editor.h"

uint64_t NVword(unsigned long int address) {
    uint64_t operation = (uint64_t)*((unsigned long int *)address);
    return operation;
}

char nvout[1000];

char *decodeNV(unsigned long int address, char *outbuf, int *lendis, unsigned long int offset0)
{
    struct editor *e = editor();
    unsigned long int start = address, i;
    unsigned long int finish = address + 2;
    uint64_t operation = NVword(start);
    for (i = 0; i < 1000; i++) nvout[i] = 0;

    if (operation == 0) { sprintf(nvout, "%s ", ""); }

    memcpy(outbuf,nvout,strlen(nvout));
    outbuf[strlen(nvout)] = '\0';
    *lendis = finish - start;
    return outbuf;
}
