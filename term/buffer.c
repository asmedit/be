#include <assert.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>

#include "buffer.h"

struct charbuf* charbuf_create() {
    struct charbuf* b = malloc(sizeof(struct charbuf));
    if (b) {
        b->contents = NULL;
        b->len = 0;
        b->cap = 0;
        return b;
    } else {
        perror("Unable to allocate size for struct charbuf");
        exit(1);
    }
}

void charbuf_free(struct charbuf* buf) {
    free(buf->contents);
    free(buf);
}

void charbuf_append(struct charbuf* buf, const char* what, size_t len) {
    assert(what != NULL);

    if ((int)(buf->len + len) >= buf->cap) {
        buf->cap += len;
        buf->cap *= 2;
        buf->contents = realloc(buf->contents, buf->cap);
        if (buf->contents == NULL) { perror("Unable to realloc charbuf"); exit(1); }
    }

    memcpy(buf->contents + buf->len, what, len);
    buf->len += len;
}

int charbuf_appendf(struct charbuf* buf, const char* fmt, ...) {
    assert(strlen(fmt) < CHARBUF_APPENDF_SIZE);

    char buffer[CHARBUF_APPENDF_SIZE];
    va_list ap;
    va_start(ap, fmt);
    int len = vsnprintf(buffer, sizeof(buffer), fmt, ap);
    va_end(ap);

    charbuf_append(buf, buffer, len);
    return len;
}

void charbuf_draw(struct charbuf* buf) {
    if (write(STDOUT_FILENO, buf->contents, buf->len) == -1) {
        perror("Can't write charbuf");
        exit(1);

    }
}

