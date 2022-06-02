#ifndef HX_BUFFER_H
#define HX_BUFFER_H

#include <stdlib.h> // size_t

static const unsigned int CHARBUF_APPENDF_SIZE = 1024;
struct charbuf { char* contents; int len; int cap; };

struct charbuf* charbuf_create();
void charbuf_free(struct charbuf* buf);
void charbuf_append(struct charbuf* buf, const char* what, size_t len);
int charbuf_appendf(struct charbuf* buf, const char* what, ...);
void charbuf_draw(struct charbuf* buf);

#endif // HX_CHARBUF_H
