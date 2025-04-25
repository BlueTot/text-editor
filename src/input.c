#include "input.h"

char *editorPrompt(char *prompt, void (*callback)(char *, int)) {
    size_t bufsize = 128;
    char *buf = malloc(bufsize);

    size_t buflen = 0;
    buf[0] = '\0';

    while (1) {
        editorSetStatusMessage(prompt, buf);
        editorRefreshScreen();

        int c = editorReadKey();
        if (c == DEL_KEY || c == CTRL_KEY('h') || c == BACKSPACE) {
            if (buflen != 0)
                buf[--buflen] = '\0';
        } else if (c == '\x1b') {
            editorSetStatusMessage("");
            if (callback)
                callback(buf, c);
            free(buf);
            return NULL;
        } else if (c == '\r') {
            if (buflen != 0) {
                editorSetStatusMessage("");
                if (callback)
                    callback(buf, c);
                return buf;
            }
        } else if (!iscntrl(c) && c < 128) {
            if (buflen == bufsize - 1) {
                bufsize *= 2;
                buf = realloc(buf, bufsize);
            }
            buf[buflen++] = c;
            buf[buflen] = '\0';
        }

        if (callback)
            callback(buf, c);
    }
}

/* Function to move cursor */
void editorMoveCursor(int key) {
    erow *row = (E.cy >= E.numrows) ? NULL : &E.row[E.cy];

    switch (key) {
        case ARROW_LEFT:
            if (E.cx != 0) {
                E.cx--;
            }
            // } else if (E.cy > 0) {
            //     E.cy--;
            //     E.cx = E.row[E.cy].size;
            // }
            break;
        case ARROW_RIGHT:
            if (row && E.cx < row->size) {
                E.cx++;
            }
            // else if (row && E.cx == row->size) {
            //     E.cy++;
            //     E.cx = 0;
            // }
            break;
        case ARROW_UP:
            if (E.cy != 0)
                E.cy--;
            break;
        case ARROW_DOWN:
            if (E.cy < E.numrows)
                E.cy++;
            break;
    }

    row = (E.cy >= E.numrows) ? NULL : &E.row[E.cy];
    int rowlen = row ? row->size : 0;
    if (E.cx > rowlen) {
        E.cx = rowlen;
    }
}

void editorProcessNormalKeypress(int c) {
    static int quit_times = KILO_QUIT_TIMES;

    switch (c) {

        // insert before cursor
        case 'i':
            E.mode = MD_INSERT;
            break;

        // insert after cursor
        case 'a':
            editorMoveCursor(ARROW_RIGHT);
            E.mode = MD_INSERT;
            break;

        // quit
        case CTRL_KEY('q'):
            if (E.dirty && quit_times > 0) {
                editorSetStatusMessage("WARNING!!! File has unsaved changes. "
                                       "Press Ctrl-Q %d more times to quit.",
                                       quit_times);
                quit_times--;
                return;
            }
            write(STDOUT_FILENO, "\x1b[2J", 4); // clear screen
            write(STDOUT_FILENO, "\x1b[H", 3);  // move cursor
            exit(0);
            break;

            // save file
        case CTRL_KEY('s'):
            editorSave();
            break;

            // home key
        case '0':
            E.cx = 0;
            break;

            // end key
        case '$':
            if (E.cy < E.numrows)
                E.cx = E.row[E.cy].size - 1;
            break;

        // find key
        case CTRL_KEY('f'):
            editorFind();
            break;

            // back space
        case BACKSPACE:
        case CTRL_KEY('h'):
        case DEL_KEY:
            editorMoveCursor(ARROW_LEFT);
            // if (c == DEL_KEY)
            //     editorMoveCursor(ARROW_RIGHT);
            // editorDelChar();
            break;

            // page up or page down keys entered
        case PAGE_UP: // fall down
        case PAGE_DOWN: {
            if (c == PAGE_UP) {
                E.cy = E.rowoff;
            } else if (c == PAGE_DOWN) {
                E.cy = E.rowoff + E.screenrows - 1;
            }

            int times = E.screenrows;
            while (times--) {
                editorMoveCursor(c == PAGE_UP ? ARROW_UP : ARROW_DOWN);
            }
        } break;

            // when we match w/a/s/d
        case ARROW_UP:   // fall down
        case ARROW_DOWN: // fall down
        case ARROW_LEFT: // fall down
        case ARROW_RIGHT:
            editorMoveCursor(c);
            break;

            // control key
        case CTRL_KEY('l'):
        case '\x1b':
            break;

            // otherwise, try to insert a character
        default:
            // editorInsertChar(c);
            break;
    }

    quit_times = KILO_QUIT_TIMES;
}

void editorProcessInsertKeypress(int c) {

    switch (c) {

        case '\x1b':
            E.mode = MD_NORMAL;
            break;

        // new line
        case '\r':
            editorInsertNewLine();
            break;

            //     // home key
            // case HOME_KEY:
            //     E.cx = 0;
            //     break;

            //     // end key
            // case END_KEY:
            //     if (E.cy < E.numrows)
            //         E.cx = E.screencols - 1;
            //     break;

            // // find key
            // case CTRL_KEY('f'):
            //     editorFind();
            //     break;

            // back space
        case BACKSPACE:
        case CTRL_KEY('h'):
        case DEL_KEY:
            if (c == DEL_KEY)
                editorMoveCursor(ARROW_RIGHT);
            editorDelChar();
            break;

            //     // page up or page down keys entered
            // case PAGE_UP: // fall down
            // case PAGE_DOWN: {
            //     if (c == PAGE_UP) {
            //         E.cy = E.rowoff;
            //     } else if (c == PAGE_DOWN) {
            //         E.cy = E.rowoff + E.screenrows - 1;
            //     }

            //     int times = E.screenrows;
            //     while (times--) {
            //         editorMoveCursor(c == PAGE_UP ? ARROW_UP : ARROW_DOWN);
            //     }
            // } break;

            // when we match w/a/s/d
        case ARROW_UP:   // fall down
        case ARROW_DOWN: // fall down
        case ARROW_LEFT: // fall down
        case ARROW_RIGHT:
            editorMoveCursor(c);
            break;

            // otherwise, try to insert a character
        default:
            editorInsertChar(c);
            break;
    }
}

/* Function to process key from user */
void editorProcessKeypress() {
    // static int quit_times = KILO_QUIT_TIMES;

    int c = editorReadKey();

    switch (E.mode) {
        case MD_NORMAL:
            editorProcessNormalKeypress(c);
            break;
        case MD_INSERT:
            editorProcessInsertKeypress(c);
            break;
    }
}