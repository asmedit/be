#include <stdint.h>
#include <string.h>

#include "sh4dis.h"
#include "sh4asm_txt_emit.h"

#define SH4ASM_TXT_LEN 228
static char sh4asm_disas[SH4ASM_TXT_LEN];
unsigned sh4asm_disas_len;
static void clear_asm(void) { sh4asm_disas_len = 0; }
static void neo_asm_emit(char ch) {
    if (sh4asm_disas_len < SH4ASM_TXT_LEN)
        sh4asm_disas[sh4asm_disas_len++] = ch;
}

char * decodeSH4(unsigned long int address, char *outbuf, int *lendis, unsigned long int offset0)
{
     uint16_t inst16 = (uint16_t)*((unsigned long int *)address);
     memset(sh4_buf, 0, sizeof(sh4_buf));
     clear_asm();
     sh4asm_disas_inst(inst16, neo_asm_emit, 0);
     memcpy(outbuf,sh4asm_disas,sh4asm_disas_len);
     memcpy(outbuf+sh4asm_disas_len,"\0",1);
     *lendis = 2;
}
