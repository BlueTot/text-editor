/*** Includes ***/

#include <unistd.h>
#include <termios.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <errno.h>

/*** Data ***/

// the original terminal settings
struct termios orig_termios;

/*** Terminal ***/

/**
 * Function to print error message
 */
void die(const char *s) {
    perror(s);
    exit(1);
}

/**
 * Function to disable raw mode for the text editor, upon exiting
 */
void disableRawMode() {
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1) {
        die("tcsetattr");
    }
}

/**
 * Function to enable raw mode for the text editor, upon entering
 */
void enableRawMode() {

    // get terminal settings and check for error
    if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) {
        die("tcgetattr");
    }

    // upon exit, disable raw mode
    atexit(disableRawMode);

    // make new terminal setting set to previous setting
    struct termios raw = orig_termios;

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

/*** Init ***/
int main() {
    enableRawMode();

    while (1) {
        char c = '\0';
        if (read(STDIN_FILENO, &c, 1) == -1 && errno != EAGAIN) {
            die("read");
        }
        if (iscntrl(c)) {
            printf("%d\r\n", c);
        } else {
            printf("%d ('%c')\r\n", c, c);
        }
        if (c == 'q') break;
    }
    return 0;
}
