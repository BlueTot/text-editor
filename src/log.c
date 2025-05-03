#include "log.h"

static FILE *logfile = NULL;

void initLog() { logfile = fopen("debug.log", "w"); }

void debugf(const char *fmt, ...) {
    if (!logfile)
        return;
    va_list args;
    va_start(args, fmt);
    vfprintf(logfile, fmt, args);
    va_end(args);
    fflush(logfile);
}

void closeLog() {
    if (logfile)
        fclose(logfile);
}
