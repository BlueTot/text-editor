/*** Includes ***/

#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#define _GNU_SOURCE

#include <unistd.h>
#include <termios.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <string.h>

/*** Defines ***/

#define KILO_VERSION "0.0.1"
#define CTRL_KEY(k) ((k) & 0x1f)

enum editorKey {
    ARROW_LEFT = 1000,
    ARROW_RIGHT,
    ARROW_UP,
    ARROW_DOWN,
    DEL_KEY,
    HOME_KEY,
    END_KEY,
    PAGE_UP,
    PAGE_DOWN
};

/*** Data ***/

typedef struct erow {
    int size;
    char *chars;
} erow;


struct editorConfig {
    int cx, cy; // cursor x, y
    int screenrows;
    int screencols;
    int numrows;
    erow *row;
    struct termios orig_termios; // original terminal settings
};

struct editorConfig E;

/*** Terminal ***/

/* Function to print error message and exit */
void die(const char *s) {
    write(STDOUT_FILENO, "\x1b[2J", 4); // clear the screen
    write(STDOUT_FILENO, "\x1b[H", 3); // move cursor to start
    perror(s);
    exit(1);
}

/* Function to disable raw mode for the text editor, upon exiting */
void disableRawMode() {
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.orig_termios) == -1) {
        die("tcsetattr");
    }
}

/* Function to enable raw mode for the text editor, upon entering */
void enableRawMode() {

    // get terminal settings and check for error
    if (tcgetattr(STDIN_FILENO, &E.orig_termios) == -1) {
        die("tcgetattr");
    }

    // upon exit, disable raw mode
    atexit(disableRawMode);

    // make new terminal setting set to previous setting
    struct termios raw = E.orig_termios;

    // set the flags for raw mode
    raw.c_iflag &= ~(BRKINT | ICRNL | IXON | INPCK | ISTRIP);
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag |= (CS8);
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    raw.c_cc[VMIN] = 0; // min num of bytes
    raw.c_cc[VTIME] = 1; // wait time (tenth of second)

    // update terminal settings
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
        die("tcsetattr");
    }
}

/* Function to read a key from the user */
int editorReadKey() {
    int nread;
    char c;
    while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
        if (nread == -1 && errno != EAGAIN) {
            die("read");
        }
    }

    if (c == '\x1b') {
        char seq[3];

        if (read(STDIN_FILENO, &seq[0], 1) != 1) {
            return '\x1b';
        }
        if (read(STDIN_FILENO, &seq[1], 1) != 1) {
            return '\x1b';
        }

        if (seq[0] == '[') {
            if (seq[1] >= '0' && seq[1] <= '9') {
                if (read(STDIN_FILENO, &seq[2], 1) != 1) {
                    return '\x1b';
                }
                if (seq[2] == '~') {
                    switch (seq[1]) {
                        case '1': return HOME_KEY;
                        case '3': return DEL_KEY;
                        case '4': return END_KEY;
                        case '5': return PAGE_UP;
                        case '6': return PAGE_DOWN;
                        case '7': return HOME_KEY;
                        case '8': return END_KEY;
                    }
                }
            } else {
                switch (seq[1]) {
                    case 'A': return ARROW_UP;
                    case 'B': return ARROW_DOWN;
                    case 'C': return ARROW_RIGHT;
                    case 'D': return ARROW_LEFT;
                    case 'H': return HOME_KEY;
                    case 'F': return END_KEY;
                }
            }
        } else if (seq[0] == '0') {
            switch (seq[1]) {
                case 'H': return HOME_KEY;
                case 'F': return END_KEY;
            }
        }

        return '\x1b';
    } else {
        return c;
    }
}

/* Function to get the cursor position */
int getCursorPosition(int *rows, int *cols) {
    // make a new buffer of characters to store a string
    char buf[32];
    unsigned int i = 0;

    // query cursor position
    if (write(STDOUT_FILENO, "\x1b[6n", 4) != 4) {
        return -1;
    }

    // read in characters and store in buffer
    while (i < sizeof(buf) - 1) {
        if (read(STDIN_FILENO, &buf[i], 1) != 1) {
            break;
        }
        if (buf[i] == 'R') {
            break;
        }
        i++;
    }
    // end string with \0
    buf[i] = '\0';

    // if string doesnt start with <esc>[, then we return -1
    if (buf[0] != '\x1b' || buf[1] != '[') {
        return -1;
    }

    // parse buf[2] by semicolon delimiter and put into rows, cols
    if (sscanf(&buf[2], "%d;%d", rows, cols) != 2) {
        return -1;
    }

    // no errors, return 0
    return 0;
}

/* Function to get the window size
 * Parameters: rows - pointer to int storing num of rows
 *             cols - pointer to int storing num of cols
 * Return: -1 if failed, 0 otherwise
 */
int getWindowSize(int *rows, int *cols) {
    struct winsize ws;

    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
        if (write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12) != 12) {
            return -1;
        }
        return getCursorPosition(rows, cols);
    } else {
        *cols = ws.ws_col;
        *rows = ws.ws_row;
        return 0;
    }
}

/*** Row Operations ***/

/* Function to append row to editor */
void editorAppendRow(char *s, size_t len) {
    E.row = realloc(E.row, sizeof(erow) * (E.numrows + 1));

    int at = E.numrows;
    E.row[at].size = len;
    E.row[at].chars = malloc(len + 1);
    memcpy(E.row[at].chars, s, len);
    E.row[at].chars[len] = '\0';
    E.numrows++;
}

/*** File I/O ***/

/* Open the edtior for reading a file from the disk */
void editorOpen(char *filename) {
    FILE *fp = fopen(filename, "r"); // open the file in read mode
    if (!fp) die ("fopen");

    char *line = "NULL";
    size_t linecap = 0;
    ssize_t linelen;
    while ((linelen = getline(&line, &linecap, fp)) != -1) {
        while (linelen > 0 && (line[linelen - 1] == '\n' ||
                               line[linelen - 1] == '\r'))
            linelen--;
        editorAppendRow(line, linelen);
    }
    free(line); // free memory
    fclose(fp); // close the file
}

/*** Append Buffer ***/

struct abuf {
    char *b;
    int len;
};

#define ABUF_INIT {NULL, 0}

/* Method to append a string to an the append buffer */
void abAppend(struct abuf *ab, const char *s, int len) {
    char *new = realloc(ab->b, ab->len + len);

    if (new == NULL) return;

    memcpy(&new[ab->len], s, len);
    ab->b = new;
    ab->len += len;
}

/* Method to free memory of append buffer */
void abFree(struct abuf *ab) {
    free(ab->b);
}

/*** Output ***/

/* Function to draw rows to the screen */
void editorDrawRows(struct abuf *ab) {
    int y;
    for (y = 0; y < E.screenrows; y++) {

        // display welcome message
        if (y >= E.numrows) {
            if (E.numrows == 0 && y == E.screenrows / 3) {

                // make welcome message
                char welcome[80];
                int welcomelen = snprintf(welcome, sizeof(welcome),
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
            int len = E.row[y].size;
            if (len > E.screencols) len = E.screencols;
            abAppend(ab, E.row[y].chars, len);
        }

        // erases part of line to the right of the cursor
        abAppend(ab, "\x1b[K", 3);
        if (y < E.screenrows - 1) {
            abAppend(ab, "\r\n", 2);
        }
    }
}

/* Function to refresh the screen */
void editorRefreshScreen() {
    struct abuf ab = ABUF_INIT;

    abAppend(&ab, "\x1b[?25l", 6); // cursor hide
    abAppend(&ab, "\x1b[H", 3); // move cursor

    editorDrawRows(&ab); // draw the rows

    char buf[32];
    // move cursor to position given by E.cx, E.cy
    snprintf(buf, sizeof(buf), "\x1b[%d;%dH", E.cy + 1, E.cx + 1);
    abAppend(&ab, buf, strlen(buf));

    abAppend(&ab, "\x1b[?25h", 6); // cursor show

    write(STDOUT_FILENO, ab.b, ab.len); // write the dynamic string to screen
    abFree(&ab); // free the memory
}

/*** Input ***/

/* Function to move cursor */
void editorMoveCursor(int key) {
    switch (key) {
        case ARROW_LEFT:
            if(E.cx != 0) E.cx--;
            break;
        case ARROW_RIGHT:
            if (E.cx != E.screencols - 1) E.cx++;
            break;
        case ARROW_UP:
            if (E.cy != 0) E.cy--;
            break;
        case ARROW_DOWN:
            if (E.cy != E.screenrows - 1) E.cy++;
            break;
    }
}

/* Function to process key from user */
void editorProcessKeypress() {
    int c = editorReadKey();

    switch (c) {
        case CTRL_KEY('q'):
            write(STDOUT_FILENO, "\x1b[2J", 4); // clear screen
            write(STDOUT_FILENO, "\x1b[H", 3); // move cursor
            exit(0);
            break;

        // home key
        case HOME_KEY:
            E.cx = 0;
            break;

        // end key
        case END_KEY:
            E.cx = E.screencols - 1;
            break;

        // page up or page down keys entered
        case PAGE_UP: // fall down
        case PAGE_DOWN:
            {
                int times = E.screenrows;
                while (times--) {
                    editorMoveCursor(c == PAGE_UP ? ARROW_UP : ARROW_DOWN);
                }
            }
            break;

        // when we match w/a/s/d
        case ARROW_UP: // fall down
        case ARROW_DOWN: // fall down
        case ARROW_LEFT: // fall down
        case ARROW_RIGHT:
            editorMoveCursor(c);
            break;
    }
}

/*** Init ***/

/* Start up the editor by initialising the struct */
void initEditor() {
    E.cx = 0;
    E.cy = 0;
    E.numrows = 0;
    E.row = NULL;

    if (getWindowSize(&E.screenrows, &E.screencols) == -1) {
        die("getWindowSize");
    }
}

/* Main function */
int main(int argc, char *argv[]) {
    enableRawMode();
    initEditor();
    if (argc >= 2) {
        editorOpen(argv[1]);
    }

    while (1) {
        editorRefreshScreen();
        editorProcessKeypress();
    }

    return 0;
}
