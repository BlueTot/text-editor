#pragma once
#include "append_buffer.h"
#include "data.h"
#include "row_ops.h"
#include "stdarg.h"
#include "syntax.h"
#include "unistd.h"
#include <ctype.h>
#include <stdio.h>

void editorScroll();
void editorDrawRows(struct abuf *ab);
void editorDrawStatusBar(struct abuf *ab);
void editorDrawMessageBar(struct abuf *ab);
void editorRefreshScreen();
void editorSetStatusMessage(const char *fmt, ...);
