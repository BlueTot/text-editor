#include "yank_buffer.h"
#include "log.h"
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

    while (r != E.schar_ey || c != E.schar_ex) {

        // store into buffer
       E.yank_buffer[buflen++] = E.row[r].chars[c];

        // if we run out of space, resize the array
        if (buflen == bufcap) {
            bufcap *= 2;
            E.yank_buffer = realloc(E.yank_buffer, bufcap);
            if (!E.yank_buffer)
                return;
        }

        // go to next position
        if (c == E.row[r].size - 1) {
            E.yank_buffer[buflen++] = '\n';
            r++;
            c = 0;
        } else {
            c++;
        }
    }

    E.yank_buffer = realloc(E.yank_buffer, ++buflen);
    E.yank_buffer[buflen - 1] = '\0';

    debugf("%d\n", buflen);
}

void pasteFromBuffer() {

    if (!E.yank_buffer)
        return;

    int r = E.cy;
    int c = E.cx;
    int startC = E.cx;
    int bufcap = 100;
    int buflen = 0;
    char *line = malloc(bufcap);

    int rowOff = 0;
    int length = strlen(E.yank_buffer);
    debugf("%d\n", length);

    // debugging code
    debugf("%s\n", E.yank_buffer);

    for (int i = 0; i < length ; i++) {

        debugf("%d\n", i);

        // put into current row
        line[buflen++] = E.yank_buffer[i];

        // resize the array
        if (buflen == bufcap) {
            bufcap *= 2;
            line = realloc(line, bufcap);
        }

        // if we reach the end of a row, we either append to the existing row or
        // insert as a new row
        if (c == E.row[r].rsize - 1 || i == length - 1) {
            // line = realloc(line, ++buflen);
            // line[buflen - 1] = '\0';

            debugf("%s\n", line);

            if (startC != 0 || i == length - 1) {
                editorRowInsertString(&E.row[r], startC, line, buflen);
                editorUpdateRow(&E.row[r]);
            } else {
                editorInsertRow(r + rowOff, line, buflen);
                editorUpdateRow(&E.row[r + rowOff]);
            }

            r++; // update row
            c = 0; // update column
            rowOff++; // increment row off
            free(line); // free the current line
            bufcap = 100; // reset buffer cap
            buflen = 0; // reset buffer length
            startC = 0; // put starting column back to 0
            line = malloc(bufcap); // create dynamic string
        } else {
            c++; // just increment column
        }
    }

    free(line);
}

void freeYankBuffer() { free(E.yank_buffer); }
