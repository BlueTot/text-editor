#pragma once
#include <stdarg.h>
#include <stdio.h>

void initLog();
void debugf(const char *fmt, ...);
void closeLog();
