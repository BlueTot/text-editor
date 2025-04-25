#pragma once
#include "data.h"
#include "editor_ops.h"
#include "file_io.h"
#include "find.h"
#include "output.h"
#include "row_ops.h"
#include "stdlib.h"
#include "terminal.h"
#include "unistd.h"
#include <ctype.h>

char *editorPrompt(char *prompt, void (*callback)(char *, int));
void editorMoveCursor(int key);
int compareCoord(int sx, int sy, int ex, int ey);
void swap(int *a, int *b);
void editorMoveVisualSelection(int key);
void editorProcessNormalKeypress(int key);
void editorProcessInsertKeypress(int key);
void editorProcessVisualCharKeypress(int key);
void editorProcessKeypress();