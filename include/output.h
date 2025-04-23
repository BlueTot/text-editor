#pragma once
#include "append_buffer.h"
#include "data.h"
#include "stdarg.h"
#include "unistd.h"

void editorScroll();
void editorDrawRows(struct abuf *ab);
void editorDrawStatusBar(struct abuf *ab);
void editorDrawMessageBar(struct abuf *ab);
void editorRefreshScreen();
void editorSetStatusMessage(const char *fmt, ...);
