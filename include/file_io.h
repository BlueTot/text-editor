#pragma once
#define _POSIX_C_SOURCE 200809L
#include "data.h"
#include "input.h"
#include "output.h"
#include "row_ops.h"
#include "syntax.h"
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <terminal.h>
#include <unistd.h>

char *editorRowsToString(int *buflen);
void editorOpen(char *filename);
void editorSave();