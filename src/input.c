#include "input.h"
#include "yank_buffer.h"

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
            switch (E.mode) {
                case MD_INSERT:
                    if (row && E.cx < row->size)
                        E.cx++;
                    break;
                default:
                    if (row && E.cx < row->size-1)
                        E.cx++;
                    break;
            }
            // }
            // if (row && E.cx < row->size - 1)
            //     E.cx++;
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

/* Function to move the cursor to the start of the line */
void editorMoveStartLine() {
    E.cx = 0;
}

/* Function to move the cursor to the end of the line */
void editorMoveEndLine() {
    if (E.cy < E.numrows)
        E.cx = E.row[E.cy].size - 1;
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

/* Function to swap two ints */
void swap(int *a, int *b) {
    int tmp = *a;
    *a = *b;
    *b = tmp;
}

/* Function to update visual character selection */
void editorUpdateVisualSelection() {
    // assign to endpoint
    E.schar_ex = E.cx;
    E.schar_ey = E.cy;
}

/* Function to update visual line selection */
void editorUpdateVisualLineSelection() {
    // assign to endpoint
    E.schar_ex = E.row[E.cy].rsize - 1;
    E.schar_ey = E.cy;

    if (compareCoord(E.schar_sx, E.schar_sy, E.schar_ex, E.schar_ey) == -1) {
        E.schar_sx = E.row[E.schar_sy].rsize - 1;
        E.schar_ex = 0;
    } else {
        E.schar_sx = 0;
        E.schar_ex = E.row[E.schar_ey].rsize - 1;
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
            E.mode = MD_INSERT;
            editorMoveCursor(ARROW_RIGHT);
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
    
        // enter visual line mode
        case 'V':
            E.mode = MD_VISUAL_LINE;
            E.is_selected = 1;
            E.schar_sx = 0;
            E.schar_sy = E.cy;
            E.schar_ex = E.row[E.cy].rsize - 1;
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
            editorMoveStartLine();
            break;

            // end key
        case '$':
            editorMoveEndLine();
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

                        // move to top of document
        case 'g':
                        E.cy = 0;
                        break;

                        // move to bottom of document
        case 'G':
                        E.cy = E.numrows - 1;
                        break;

                        // control key
        case CTRL_KEY('l'):
        case '\x1b':
                        break;

                        // paste from buffer
        case 'p':
                        pasteFromBuffer();
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
            editorMoveCursor(ARROW_UP);
            editorUpdateVisualSelection();
            break;

            // move down
        case ARROW_DOWN:
        case 'j':
            editorMoveCursor(ARROW_DOWN);
            editorUpdateVisualSelection();
            break;

            // move left
        case ARROW_LEFT:
        case 'h':
            editorMoveCursor(ARROW_LEFT);
            editorUpdateVisualSelection();
            break;

            // move right
        case ARROW_RIGHT:
        case 'l':
            editorMoveCursor(ARROW_RIGHT);
            editorUpdateVisualSelection();
            break;

            // move to top of document
        case 'g':
            E.cy = 0;
            editorUpdateVisualSelection();
            break;

            // move to bottom of document
        case 'G':
            E.cy = E.numrows - 1;
            editorUpdateVisualSelection();
            break;

            // home key
        case '0':
            editorMoveStartLine();
            editorUpdateVisualSelection();
            break;

            // end key
        case '$':
            editorMoveEndLine();
            editorUpdateVisualSelection();
            break;

            // yank into buffer
        case 'y':
            yankToBuffer();
            E.mode = MD_NORMAL;
            E.is_selected = 0;
            break;
    }
}


void editorProcessVisualLineKeypress(int c) {

    switch (c) {

        // escape key
        case '\x1b':
            E.mode = MD_NORMAL;
            E.is_selected = 0;
            break;

            // move up
        case ARROW_UP:
        case 'k':
            editorMoveCursor(ARROW_UP);
            editorUpdateVisualLineSelection();
            break;

            // move down
        case ARROW_DOWN:
        case 'j':
            editorMoveCursor(ARROW_DOWN);
            editorUpdateVisualLineSelection();
            break;

            // move left
        case ARROW_LEFT:
        case 'h':
            editorMoveCursor(ARROW_LEFT);
            editorUpdateVisualLineSelection();
            break;

            // move right
        case ARROW_RIGHT:
        case 'l':
            editorMoveCursor(ARROW_RIGHT);
            editorUpdateVisualLineSelection();
            break;

            // move to top of document
        case 'g':
            E.cy = 0;
            editorUpdateVisualLineSelection();
            break;

            // move to bottom of document
        case 'G':
            E.cy = E.numrows - 1;
            editorUpdateVisualLineSelection();
            break;

            // home key
        case '0':
            editorMoveStartLine();
            editorUpdateVisualLineSelection();
            break;

            // end key
        case '$':
            editorMoveEndLine();
            editorUpdateVisualLineSelection();
            break;

            // yank into buffer
        case 'y':
            yankToBuffer();
            E.mode = MD_NORMAL;
            E.is_selected = 0;
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
        case MD_VISUAL_LINE:
            editorProcessVisualLineKeypress(c);
    }
}
