/*** Includes ***/

#include <unistd.h>
#include <termios.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <errno.h>
#include <sys/ioctl.h>

/*** Defines ***/
#define CTRL_KEY(k) ((k) & 0x1f)

/*** Data ***/

struct editorConfig {
    int screenrows;
    int screencols;
    struct termios orig_termios;
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
char editorReadKey() {
    int nread;
    char c;
    while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
        if (nread == -1 && errno != EAGAIN) {
            die("read");
        }
    }
    return c;
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

/*** Output ***/

/* Function to draw rows to the screen */
void editorDrawRows() {
    int y;
    for (y = 0; y < E.screenrows; y++) {
         write(STDOUT_FILENO, "~", 1);

         if (y < E.screenrows - 1) {
             write(STDOUT_FILENO, "\r\n", 2);
         }
    }
}

/* Function to refresh the screen */
void editorRefreshScreen() {
    write(STDOUT_FILENO, "\x1b[2J", 4); // clear the screen
    write(STDOUT_FILENO, "\x1b[H", 3); // move cursor to start
    editorDrawRows();
    write(STDOUT_FILENO, "\x1b[H", 3); // move cursor to start
}

/*** Input ***/

/* Function to process key from user */
void editorProcessKeypress() {
    char c = editorReadKey();

    switch (c) {
        case CTRL_KEY('q'):
            exit(0);
            break;
    }
}

/*** Init ***/

/* Start up the editor by initialising the struct */
void initEditor() {
    if (getWindowSize(&E.screenrows, &E.screencols) == -1) {
        die("getWindowSize");
    }
}

/* Main function */
int main() {
    enableRawMode();
    initEditor();

    while (1) {
        editorRefreshScreen();
        editorProcessKeypress();
    }
    return 0;
}
