#include <stdint.h>
#include <string.h>

#include "sh4dis.h"
#include "sh4asm_txt_emit.h"

char * decodeSH4(unsigned long int address, char *outbuf, int *lendis);
