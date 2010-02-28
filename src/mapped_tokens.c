#include "standard.h"
#include "mapped_tokens.h"
#include "util.h"
#include "log.h"

#pragma pack(push, 1)

typedef struct {
    int                     map_size;
    int const *             token_idx;
    int                     ntokens;
    int const *             str_offset;
    int const *             indices_offset;
    int const *             indices_num;
    int                     names_len;
    char const *            names;
    int                     indices_len;
    int const *             indices;
} mapped_tokens_t;

#pragma pack(pop)

static mapped_tokens_t tokens;

void init_mapped_tokens(void const *p) {
    char const *ptr = p;

    tokens.map_size = *(int *) ptr;
    ptr += sizeof tokens.map_size;
    debug("%d entries in token map\n", tokens.map_size);

    tokens.token_idx = (int *) ptr;
    ptr += sizeof(int) * tokens.map_size;
    debug("%dMB size of token map\n", (tokens.map_size * sizeof(int)) >> 20);

    tokens.ntokens = *(int *) ptr;
    ptr += sizeof tokens.ntokens;
    debug("%d tokens\n", tokens.ntokens);

    tokens.str_offset = (int const *) ptr;
    ptr += sizeof(int) * tokens.ntokens;
    debug("%dMB for string offsets\n", (sizeof(int) * tokens.ntokens) >> 20);

    tokens.indices_offset = (int const *) ptr;
    ptr += sizeof(int) * tokens.ntokens;
    debug("%dMB for indices offsets\n", (sizeof(int) * tokens.ntokens) >> 20);

    tokens.indices_num = (int const *) ptr;
    ptr += sizeof(int) * tokens.ntokens;
    debug("%dMB for number of indices per token\n", (sizeof(int) * tokens.ntokens) >> 20);

    tokens.names_len = *(int *) ptr;
    ptr += sizeof tokens.names_len;

    tokens.names = (char const *) ptr;
    ptr += tokens.names_len;
    debug("%dMB for strings\n", tokens.names_len >> 20);

    tokens.indices_len = *(int *) ptr;
    ptr += sizeof tokens.indices_len;
    debug("%d indices\n", tokens.indices_len);

    tokens.indices = (int *) ptr;
    debug("%dMB for indices data\n", (sizeof(int) * tokens.indices_len) >> 20);
}

geoname_indices_t mapped_geonames_by_token(char const *token) {
    int start_pos = 0, i = 0, j = 0, mask = 0;

    mask = tokens.map_size - 1;
    start_pos = strhash(token) & mask;

    for (i = start_pos;;) {
        int token_idx = tokens.token_idx[i];

        if (token_idx != -1) {
            char const *last = tokens.names + tokens.str_offset[token_idx];
            char const *first = last;

            while (*first)
                --first;
            ++first;

            if (!strncmp(token, first, last - first + 1)) {
                return vector_from_memory(sizeof(int), tokens.indices_num[token_idx],
                                          tokens.indices + tokens.indices_offset[token_idx]);
            }
        } else
            break;

        i = (i + (j++ << 1) + 1) & mask;
        if (i == start_pos)
            break;
    }

    return NULL;
}
