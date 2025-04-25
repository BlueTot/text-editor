#include "output.h"
#include "input.h"

/* Scroll through the editor */
void editorScroll() {
    E.rx = 0;
    if (E.cy < E.numrows) {
        E.rx = editorRowCxToRx(&E.row[E.cy], E.cx);
    }

    // scroll up
    if (E.cy < E.rowoff) {
        E.rowoff = E.cy;
    }
    // scroll down
    if (E.cy >= E.rowoff + E.screenrows) {
        E.rowoff = E.cy - E.screenrows + 1;
    }
    // scroll right
    if (E.cx - E.screencols >= 0) {
        E.coloff = E.cx - E.screencols;
    } else {
        E.coloff = 0;
    }
}

/* Function to draw rows to the screen */
void editorDrawRows(struct abuf *ab) {
    int y;
    for (y = 0; y < E.screenrows; y++) {
        int filerow = y + E.rowoff;

        if (filerow >= E.numrows) {

            // display welcome message
            if (E.numrows == 0 && y == E.screenrows / 3) {

                // make welcome message
                char welcome[80];
                int welcomelen =
                    snprintf(welcome, sizeof(welcome),
                             "Kilo editor -- version %s", KILO_VERSION);
                if (welcomelen > E.screencols) {
                    welcomelen = E.screencols;
                }

                // centering
                int padding = (E.screencols - welcomelen) / 2;
                if (padding) {
                    abAppend(ab, "~", 1);
                    padding--;
                }
                while (padding--) {
                    abAppend(ab, " ", 1);
                }

                // draw to dynamic string
                abAppend(ab, welcome, welcomelen);

                // print tildes
            } else {
                abAppend(ab, "~", 1); // draw a tilde
            }
        } else {
            int len = E.row[filerow].rsize - E.coloff;
            if (len < 0)
                len = 0;
            if (len > E.screencols)
                len = E.screencols;

            // character array
            char *c = &E.row[filerow].render[E.coloff];

            // highlighting array
            unsigned char *hl = &E.row[filerow].hl[E.coloff];
            int current_color = -1; // the current colour
            int j;                  // the rendering column index

            for (j = 0; j < len; j++) {

                // selection coloring
                int actual_col = j + E.coloff;
                if (E.is_selected &&
                    compareCoord(E.schar_sx, E.schar_sy, actual_col, y) >= 0 &&
                    compareCoord(actual_col, y, E.schar_ex, E.schar_ey) >= 0) {
                    abAppend(ab, "\x1b[47m", 5);
                }

                if (iscntrl(c[j])) { // if the character is a control character
                    char sym = (c[j] <= 26) ? '@' + c[j] : '?';
                    abAppend(ab, "\x1b[7m", 4);
                    abAppend(ab, &sym, 1);
                    abAppend(ab, "\x1b[m", 3);
                    if (current_color != -1) {
                        char buf[16];
                        int clen = snprintf(buf, sizeof(buf), "\x1b[%dm",
                                            current_color);
                        abAppend(ab, buf, clen);
                    }
                } else if (hl[j] ==
                           HL_NORMAL) { // if the character has normal color
                    if (current_color !=
                        -1) { // if the previous color is not normal
                        abAppend(ab, "\x1b[39m", 5); // clear color
                        current_color = -1;
                    }
                    abAppend(ab, &c[j], 1);
                } else { // otherwise it is colored
                    int color = editorSyntaxToColor(hl[j]);
                    if (color !=
                        current_color) {       // if previous color is different
                        current_color = color; // change the color
                        char buf[16];
                        int clen = snprintf(buf, sizeof(buf), "\x1b[%dm",
                                            color); // write to buffer
                        abAppend(ab, buf, clen);
                    }
                    abAppend(ab, &c[j], 1); // append character
                }

                abAppend(ab, "\x1b[49m", 5); // clear background
            }
            abAppend(ab, "\x1b[39m", 5); // clear color
        }

        // erases part of line to the right of the cursor
        abAppend(ab, "\x1b[K", 3);
        abAppend(ab, "\r\n", 2);
    }
}

/* Function to draw status bar */
void editorDrawStatusBar(struct abuf *ab) {

    // enter inverted mode
    abAppend(ab, "\x1b[7m", 4);

    // make buffer for status bar, and line number status bar
    char status[80], rstatus[80];
    int len = snprintf(status, sizeof(status), " [%s] | %.20s - %d lines %s",
                       E.mode == MD_NORMAL
                           ? "NORMAL"
                           : (E.mode == MD_INSERT ? "INSERT" : "VISUAL"),
                       E.filename ? E.filename : "[No Name]", E.numrows,
                       E.dirty ? "(modified)" : "");
    int rlen =
        snprintf(rstatus, sizeof(rstatus), "%s | %d/%d",
                 E.syntax ? E.syntax->filetype : "no ft", E.cy + 1, E.numrows);

    if (len > E.screencols)
        len = E.screencols;
    abAppend(ab, status, len);

    while (len < E.screencols) {
        // once we are aligned against right edge of screen
        if (E.screencols - len == rlen) {
            abAppend(ab, rstatus, rlen); // add the second status bar
            break;
        } else {
            abAppend(ab, " ", 1); // add space
            len++;
        }
    }

    // restore settings
    abAppend(ab, "\x1b[m", 3);
    abAppend(ab, "\r\n", 2);
}

/* Function to draw message bar */
void editorDrawMessageBar(struct abuf *ab) {
    abAppend(ab, "\x1b[K", 3); // clear the message bar
    int msglen = strlen(E.statusmsg);
    if (msglen > E.screencols)
        msglen = E.screencols;
    if (msglen && time(NULL) - E.statusmsg_time < 5)
        abAppend(ab, E.statusmsg, msglen);
}

/* Function to refresh the screen */
void editorRefreshScreen() {
    editorScroll();

    struct abuf ab = ABUF_INIT;

    abAppend(&ab, "\x1b[?25l", 6); // cursor hide
    abAppend(&ab, "\x1b[H", 3);    // move cursor

    editorDrawRows(&ab);       // draw the rows
    editorDrawStatusBar(&ab);  // draw the status bar
    editorDrawMessageBar(&ab); // draw the message bar

    char buf[32];
    // move cursor to position given by E.cx, E.cy
    snprintf(buf, sizeof(buf), "\x1b[%d;%dH", (E.cy - E.rowoff) + 1,
             (E.rx - E.coloff) + 1);
    abAppend(&ab, buf, strlen(buf));

    if (E.mode == MD_INSERT) {
        abAppend(&ab, "\x1b[5 q", 6);
    } else {
        abAppend(&ab, "\x1b[2 q", 6);
    }

    abAppend(&ab, "\x1b[?25h", 6); // cursor show

    write(STDOUT_FILENO, ab.b, ab.len); // write the dynamic string to screen
    abFree(&ab);                        // free the memory
}

/* Set status message function - ... means it is a variadic function */
void editorSetStatusMessage(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(E.statusmsg, sizeof(E.statusmsg), fmt, ap);
    va_end(ap);
    E.statusmsg_time = time(NULL);
}