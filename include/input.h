#pragma once
#include "data.h"
#include "unistd.h"

char *editorPrompt(char *prompt, void (*callback)(char *, int));
void editorMoveCursor(int key);
void editorProcessKeypress();