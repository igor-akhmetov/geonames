#include "standard.h"
#include "mapped_tokens.h"
#include "util.h"

#pragma pack(push, 1)

typedef struct {
    int size;
    int data[];
} mapped_geoindices_t;

typedef struct {
    int str_offset;
    int indices_offset;
} mapped_token_t;

typedef struct {
    int                     map_size;
    mapped_token_t const *  tokens;
    int                     names_len;
    char const *            names;
    void const *            indices;
} mapped_tokens_t;

#pragma pack(pop)

static mapped_tokens_t tokens;

void init_mapped_tokens(void const *p) {
    char const *ptr = p;

    tokens.map_size = *(int *) ptr;
    ptr += sizeof tokens.map_size;

    tokens.tokens = (mapped_token_t *) ptr;
    ptr += sizeof(mapped_token_t) * tokens.map_size;

    tokens.names_len = *(int *) ptr;
    ptr += sizeof tokens.names_len;

    tokens.names = (char const *) ptr;
    ptr += tokens.names_len;

    tokens.indices = ptr;
}

geoname_indices_t mapped_geonames_by_token(char const *token) {
    int start_pos = 0, i = 0, j = 0, size = 0;

    size = tokens.map_size;
    start_pos = strhash(token) % size;

    for (i = start_pos;;) {
        char const *last = tokens.names + tokens.tokens[i].str_offset;

        if (*last) {
            char const *first = last;

            while (*first)
                --first;
            ++first;

            if (!strncmp(token, first, last - first + 1)) {
                mapped_geoindices_t const * v = (mapped_geoindices_t const *)
                    ((char *) tokens.indices + tokens.tokens[i].indices_offset);
                if (v->size < 0)
                    return vector_from_memory(sizeof(v->data[0]), ~v->size, v->data);
                else
                    return vector_from_memory(sizeof(v->size), 1, &v->size);
            }
        } else 
            break;

        i = (i + (j << 1) + 1) % size;
        ++j;
        if (i == start_pos)
            break;
    }

    return NULL;
}