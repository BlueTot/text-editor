#include "data.h"
#include "input.h"
#include "log.h"
#include "output.h"
#include "terminal.h"

/* Start up the editor by initialising the struct */
void initEditor() {
    E.cx = 0;
    E.cy = 0;
    E.rx = 0;
    E.rowoff = 0;
    E.coloff = 0;
    E.numrows = 0;
    E.row = NULL;
    E.dirty = 0;
    E.filename = NULL;
    E.statusmsg[0] = '\0';
    E.statusmsg_time = 0;
    E.syntax = NULL;
    E.mode = MD_NORMAL;
    E.is_selected = 0;
    E.schar_sx = 0;
    E.schar_sy = 0;
    E.schar_ex = 0;
    E.schar_ey = 0;
    E.yank_buffer = NULL;

    if (getWindowSize(&E.screenrows, &E.screencols) == -1) {
        die("getWindowSize");
    }

    // make room for both status bars
    E.screenrows -= 2;
    initLog();
}

/* Main function */
int main(int argc, char *argv[]) {
    enableRawMode();
    initEditor();
    if (argc >= 2) {
        editorOpen(argv[1]);
    }

    editorSetStatusMessage(
        "HELP: Ctrl-S = save | Ctrl-Q = quit | CTRL-F = find");

    while (1) {
        editorRefreshScreen();
        editorProcessKeypress();
    }

    return 0;
}
