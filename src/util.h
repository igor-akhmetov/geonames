#pragma once

#include "vector.h"

/* Memory allocation with error checking. */
void *          xmalloc(size_t size);
void *          xcalloc(size_t size);
void *          xrealloc(void *ptr, size_t size);

/* Map read-only view of file to the process' memory. */
void const *    map_file_read(char const *filename, int populate_data);

/* String utilities. */

/* String duplication with error checking. */
char *          xstrdup(char const *src);

/* Splits a string similar to strtok, but return all the results via vector. */
vector_t        strsplit(char *str, char const *sep);

/* Lower all ASCII symbols in the given string. Doesn't handle UTF-8 symbols. */
char *          strlower(char *str);

/* Strip off '\r' and '\n' characters from the end of the string. */
char *          strtrim(char *str);

/* Calculate hash if the given string. */
unsigned        strhash(char const * key);
