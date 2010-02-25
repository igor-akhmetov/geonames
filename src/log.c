#include "standard.h"
#include "log.h"

static int verbose;
static char const *program_name;

void set_verbose(int val) {
    verbose = val;
}

void set_program_name(char const *name) {
    program_name = name;
}

void debug(char const *msg, ...) {
    va_list args;

    if (!verbose)
        return;

    va_start(args, msg);  
    vprintf(msg, args);
    va_end(args);
}

void usage(char const *msg, ...) {   
    va_list args;
    va_start(args, msg);  

    fprintf(stderr, "usage: %s ", program_name);
    vfprintf(stderr, msg, args);
    fputs("\n", stderr);

    va_end(args);    
    exit(EXIT_FAILURE);
}

void error(char const *msg, ...) {   
    va_list args;
    va_start(args, msg);  

    fprintf(stderr, "%s: ", program_name);
    vfprintf(stderr, msg, args);
    fputs("\n", stderr);

    va_end(args);    
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
