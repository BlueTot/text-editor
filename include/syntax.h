#pragma once
#include "data.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

int is_separator(int c);
void editorUpdateSyntax(erow *row);
int editorSyntaxToColor(int hl);
void editorSelectSyntaxHighlight();