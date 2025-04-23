struct editorSyntax {
    char *filetype;
    char **filematch;
    char **keywords;
    char *singleline_comment_start;
    char *multiline_comment_start;
    char *multiline_comment_end;
    int flags;
};

typedef struct erow {
    int idx;
    int size;
    int rsize;
    char *chars;
    char *render;
    unsigned char *hl;
    int hl_open_comment;
} erow;

struct editorConfig {
    int cx, cy; // cursor x, y, infile
    int rx;
    int rowoff; // row offset in file
    int coloff; // column offset in file
    int screenrows;
    int screencols;
    int numrows;
    erow *row; // the current row struct
    int dirty;
    char *filename; // filename
    char statusmsg[80];
    time_t statusmsg_time;
    struct editorSyntax *syntax;
    struct termios orig_termios; // original terminal settings
};

struct editorConfig E;