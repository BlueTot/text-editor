#include "append_buffer.h"

/* Method to append a string to an the append buffer */
void abAppend(struct abuf *ab, const char *s, int len) {
    char *new = realloc(ab->b, ab->len + len);

    if (new == NULL)
        return;

    memcpy(&new[ab->len], s, len);
    ab->b = new;
    ab->len += len;
}

/* Method to free memory of append buffer */
void abFree(struct abuf *ab) { free(ab->b); }