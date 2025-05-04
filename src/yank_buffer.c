#include "yank_buffer.h"
#include "log.h"
#include "output.h"
#include "row_ops.h"

/* Function to yank selected text into buffer */
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

    while (1) {

        // debugf("%d %d\n", r, c);

        // if we run out of space, resize the array
        if (buflen == bufcap) {
            bufcap *= 2;
            E.yank_buffer = realloc(E.yank_buffer, bufcap);
            if (!E.yank_buffer)
                return;
        }

        // store into buffer
        E.yank_buffer[buflen++] = E.row[r].chars[c];

        // if we reach the end point, break
        if (r == E.schar_ey && c == E.schar_ex)
            break;

        if (E.row[r].chars[c] == '\0') {
            E.yank_buffer[buflen++] = '\n';
            r++;
            c = 0;
        } else {
            c++;
        }
    }

    // if we do not end on a null character, append it to finish the string
    if (E.yank_buffer[buflen - 1] != '\0') {
        E.yank_buffer = realloc(E.yank_buffer, ++buflen);
        E.yank_buffer[buflen - 1] = '\0';
    }
    E.ylen = buflen;

    // debugf("%s\n", E.yank_buffer);
    // debugf("Length: %d\n", buflen);

    editorSetStatusMessage("%d chars yanked to buffer", buflen);
}

/* Function to paste text from buffer */
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

    for (int i = 0; i < E.ylen ; i++) {

        // debugf("%d %d\n", r, c);

        if (E.yank_buffer[i] == '\n' || i == E.ylen - 1) {

            if (i == E.ylen - 1)
                    line[buflen++] = E.yank_buffer[i];

            // debugf("%s %d %d\n", line, rowOff, startC);

            if (startC != 0 && i == E.ylen - 1) {
                editorRowInsertString(&E.row[r], startC, line, buflen - 1);
                editorUpdateRow(&E.row[r]);
            } else {
                editorInsertRow(r, line, buflen - 1);
                editorUpdateRow(&E.row[r]);
            }

            r++; // update row
            c = 0; // update column
            rowOff++; // increment row off
            free(line); // free the current line
            bufcap = 100; // reset buffer cap
            buflen = 0; // reset buffer length
            startC = 0; // put starting column back to 0
            line = malloc(bufcap); // create dynamic string
            continue;
        }

        // put into current row
        line[buflen++] = E.yank_buffer[i];

        // resize the array
        if (buflen == bufcap) {
            bufcap *= 2;
            line = realloc(line, bufcap);
        }
        c++;

    }

    free(line);

    editorSetStatusMessage("%d chars pasted from buffer", E.ylen);
}

void freeYankBuffer() { free(E.yank_buffer); }
