#pragma once

/* Logging interface. */

/* Toggle debug output. */
void set_verbose(int verbose);

/* Set program name to be used in error messages. */
void set_program_name(char const *name);

/* Prints usage message and terminates the program. */
void usage(char const * msg, ...);

/* Print debug info if the corresponding flag has been set. */
void debug(char const * msg, ...);

/* Prints an error message nad terminates the program. */
void error(char const * msg, ...);

/* Prints an error message followed by the string representation
   of current C error number and terminates the program. */
void cerror(char const * msg, ...);
