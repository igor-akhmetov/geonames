#pragma once

#include "vector.h"

void *          xmalloc(size_t size);
void *          xcalloc(size_t size);
void *          xrealloc(void *ptr, size_t size);

void const *    map_file_read(char const *filename);

char *          xstrdup(char const *src);
vector_t        strsplit(char *str, char const *sep);
char *          strlower(char *str);
char *          strtrim(char *str);
unsigned        strhash(char const * key);
