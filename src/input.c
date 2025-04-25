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
            if (E.cx != 0)
                E.cx--;
            break;
        case ARROW_RIGHT:
            if (row && E.cx < row->size)
                E.cx++;
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

int compareCoord(int sx, int sy, int ex, int ey) {
    if (sy < ey) {
        return 1;
    } else if (sy > ey) {
        return -1;
    } else {
        if (sx < ex) {
            return 1;
        } else if (sx > ex) {
            return -1;
        } else {
            return 0;
        }
    }
}

void swap(int *a, int *b) {
    int tmp = *a;
    *a = *b;
    *b = tmp;
}

void editorMoveVisualSelection(int key) {

    // move the cursor and update the new end point
    editorMoveCursor(key);
    E.schar_ex = E.cx;
    E.schar_ey = E.cy;

    // if the end is before the start, swap them
    if (compareCoord(E.schar_sx, E.schar_sy, E.schar_ex, E.schar_ey) == -1) {
        swap(&E.schar_sx, &E.schar_ex);
        swap(&E.schar_sy, &E.schar_ey);
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

        // enter visual character mode
        case 'v':
            E.mode = MD_VISUAL_CHAR;
            E.is_selected = 1;
            E.schar_sx = E.cx;
            E.schar_sy = E.cy;
            E.schar_ex = E.cx;
            E.schar_ey = E.cy;
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
                E.cx = E.row[E.cy].size;
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

        // move up
        case ARROW_UP:
        case 'k':
            editorMoveCursor(ARROW_UP);
            break;

        // move down
        case ARROW_DOWN:
        case 'j':
            editorMoveCursor(ARROW_DOWN);
            break;

        // move left
        case ARROW_LEFT:
        case 'h':
            editorMoveCursor(ARROW_LEFT);
            break;

        // move right
        case ARROW_RIGHT:
        case 'l':
            editorMoveCursor(ARROW_RIGHT);
            break;

        // control key
        case CTRL_KEY('l'):
        case '\x1b':
            break;

        // otherwise
        default:
            break;
    }

    quit_times = KILO_QUIT_TIMES;
}

void editorProcessInsertKeypress(int c) {

    switch (c) {

        // escape key
        case '\x1b':
            E.mode = MD_NORMAL;
            break;

        // new line
        case '\r':
            editorInsertNewLine();
            break;

        // back space
        case BACKSPACE:
        case CTRL_KEY('h'):
        case DEL_KEY:
            if (c == DEL_KEY)
                editorMoveCursor(ARROW_RIGHT);
            editorDelChar();
            break;

        // movement keys
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

void editorProcessVisualCharKeypress(int c) {

    switch (c) {

        // escape key
        case '\x1b':
            E.mode = MD_NORMAL;
            E.is_selected = 0;
            break;

        // move up
        case ARROW_UP:
        case 'k':
            editorMoveVisualSelection(ARROW_UP);
            break;

        // move down
        case ARROW_DOWN:
        case 'j':
            editorMoveVisualSelection(ARROW_DOWN);
            break;

        // move left
        case ARROW_LEFT:
        case 'h':
            editorMoveVisualSelection(ARROW_LEFT);
            break;

        // move right
        case ARROW_RIGHT:
        case 'l':
            editorMoveVisualSelection(ARROW_RIGHT);
            break;
    }
}

/* Function to process key from user */
void editorProcessKeypress() {
    int c = editorReadKey();

    switch (E.mode) {
        case MD_NORMAL:
            editorProcessNormalKeypress(c);
            break;
        case MD_INSERT:
            editorProcessInsertKeypress(c);
            break;
        case MD_VISUAL_CHAR:
            editorProcessVisualCharKeypress(c);
            break;
    }
}