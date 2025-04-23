#pragma once
#include <stdlib.h>
#include <string.h>

struct abuf {
    char *b;
    int len;
};

#define ABUF_INIT {NULL, 0}

// prototypes
void abAppend(struct abuf *ab, const char *s, int len);
void abFree(struct abuf *ab);