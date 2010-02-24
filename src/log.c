#include "standard.h"
#include "log.h"

int verbose;

void debug(char const *msg, ...)
{
    va_list args;
    va_start(args, msg);  
    vprintf(msg, args);
    va_end(args);
}

void error(char const *msg, ...)
{
    va_list args;
    va_start(args, msg);  
    vfprintf(stderr, msg, args);
    va_end(args);
    fputs("\n", stderr);
    exit(EXIT_FAILURE);
}

void cerror(char const *msg, ...) {
    char buf[1024];

    va_list args;
    va_start(args, msg);  
    vsnprintf(buf, sizeof buf, msg, args);
    va_end(args);

    error("%s: %s\n", buf, strerror(errno));
}
