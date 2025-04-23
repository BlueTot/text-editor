#pragma once
#include "data.h"
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

char *editorRowsToString(int *buflen);
void editorOpen(char *filename);
void editorSave();