#include "terminal.h"
#include "log.h"

/* Function to print error message and exit */
void die(const char *s) {
    write(STDOUT_FILENO, "\x1b[2J", 4); // clear the screen
    write(STDOUT_FILENO, "\x1b[H", 3);  // move cursor to start
    perror(s);
    closeLog();
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
    raw.c_cc[VMIN] = 0;  // min num of bytes
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
                        case '1':
                            return HOME_KEY;
                        case '3':
                            return DEL_KEY;
                        case '4':
                            return END_KEY;
                        case '5':
                            return PAGE_UP;
                        case '6':
                            return PAGE_DOWN;
                        case '7':
                            return HOME_KEY;
                        case '8':
                            return END_KEY;
                    }
                }
            } else {
                switch (seq[1]) {
                    case 'A':
                        return ARROW_UP;
                    case 'B':
                        return ARROW_DOWN;
                    case 'C':
                        return ARROW_RIGHT;
                    case 'D':
                        return ARROW_LEFT;
                    case 'H':
                        return HOME_KEY;
                    case 'F':
                        return END_KEY;
                }
            }
        } else if (seq[0] == '0') {
            switch (seq[1]) {
                case 'H':
                    return HOME_KEY;
                case 'F':
                    return END_KEY;
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