#pragma once

void set_verbose(int verbose);
void set_program_name(char const *name);

void usage(char const * msg, ...);
void debug(char const * msg, ...);
void error(char const * msg, ...);
void cerror(char const * msg, ...);
