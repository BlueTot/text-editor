#include "yank_buffer.h"
#include "row_ops.h"

void yankToBuffer() {

    // first try to free the yank buffer
    free(E.yank_buffer);
    E.yank_buffer = NULL;
    int buflen = 0;
    int bufcap = 1024; // start with 1024 first

    // allocate memory
    E.yank_buffer = malloc(bufcap);
    if (!E.yank_buffer)
        return;

    int r = E.schar_sy;
    int c = E.schar_sx;

    while (r != E.schar_sy || c != E.schar_sx) {

        // store into buffer
        E.yank_buffer[buflen++] = E.row[r].render[c];

        // if we run out of space, resize the array
        if (buflen == bufcap) {
            bufcap *= 2;
            E.yank_buffer = realloc(E.yank_buffer, bufcap);
            if (!E.yank_buffer)
                return;
        }

        // go to next position
        if (c == E.row[r].rsize - 1) {
            E.yank_buffer[buflen++] = '\n';
            r++;
            c = 0;
        } else {
            c++;
        }
    }

    E.yank_buffer = realloc(E.yank_buffer, ++buflen);
    E.yank_buffer[buflen] = '\0';
}

void pasteFromBuffer() {

    int r = E.cy;
    int c = E.cx;
    int bufcap = 100;
    int buflen = 0;
    char *line = malloc(bufcap);
    int isStart = 1;
    int rowOff = 0;

    for (int i = 0; E.yank_buffer[i] != '\0'; i++) {

        // put into current row
        line[buflen++] = E.yank_buffer[i];

        // resize the array
        if (buflen == bufcap) {
            bufcap *= 2;
            line = realloc(line, bufcap);
        }

        // if we reach the end of a row, we either append to the existing row or
        // insert as a new row
        if (c == E.row[r].rsize - 1) {
            line = realloc(line, ++buflen);
            line[buflen] = '\0';
            if (isStart) {
                editorRowAppendString(&E.row[r], line, buflen + 1);
            } else {
                editorInsertRow(r + rowOff, line, buflen + 1);
            }
            rowOff++;
            free(line);
            bufcap = 100;
            buflen = 0;
            line = malloc(bufcap);
        }
    }

    free(line);
}

void freeYankBuffer() { free(E.yank_buffer); }