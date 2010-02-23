#include "standard.h"
#include "tokens.h"
#include "geonames.h"
#include "hash_map.h"
#include "log.h"
#include "util.h"
#include "admin_codes.h"

static hash_map_t tokens;

static void add_token(char *str, geoname_idx_t geo_idx) {
    char *word = xstrdup(str);
    int len = strlen(word);

    do {
        geoname_indices_t * geonames = hash_map_get(tokens, word);
        int size;

        if (!geonames) {
            vector_t v = vector_init(sizeof(geoname_idx_t));
            geonames = (geoname_indices_t *) hash_map_put(tokens, word, &v);
        }

        size = vector_size(*geonames);
        if (!size || geoname_idx(*geonames, size - 1) != geo_idx)
            vector_push(*geonames, &geo_idx);

        word[--len] = 0;
    } while (len);

    free(word);
}

static void add_token_with_dashes(char *word, geoname_idx_t geo_idx) {
    vector_t parts;
    int i;

    if (!word)
        return;

    add_token(word, geo_idx);
    parts = strsplit(word, "-");

    for (i = 0; i != vector_size(parts); ++i)
        add_token(*(char **) vector_at(parts, i), geo_idx);

    vector_free(parts);
}

static void add_token_with_spaces(char const *str, geoname_idx_t geo_idx) {
    int j;
    char *token;
    vector_t words;

    if (!str)
        return;

    token = strlower(xstrdup(str));
    words = strsplit(token, " \t");

    for (j = 0; j != vector_size(words); ++j)
        add_token_with_dashes(*(char **)vector_at(words, j), geo_idx);

    vector_free(words);
    free(token);
}

static void add_token_with_commas(char const *str, geoname_idx_t geo_idx) {
    int j;
    char *token; 
    vector_t words;

    if (!str)
        return;

    token = strlower(xstrdup(str));
    words = strsplit(token, ",");

    for (j = 0; j != vector_size(words); ++j)
        add_token_with_spaces(*(char **)vector_at(words, j), geo_idx);

    vector_free(words);
    free(token);
}

void collect_tokens() {
    int i;

    tokens = hash_map_init(sizeof(vector_t));
    
    for (i = 0; i != geonames_num(); ++i) {
        geoname_t const *g = geoname(i);
        country_info_t const *ci = country(g->country_idx);

        if (!(i % 1000))
            debug("processing geoname %d, %d tokens so far\n", i, hash_map_size(tokens));

        add_token_with_spaces(g->name, i);
        add_token_with_commas(g->alternate_names, i);

        if (ci) {
            admin_names_t admin_names;

            add_token_with_spaces(ci->name, i);
            add_token_with_spaces(ci->fips, i);
            add_token_with_spaces(ci->iso, i);
            add_token_with_spaces(ci->iso3, i);
            
            get_admin_names(g->country_idx, g->admin1_code, g->admin2_code, &admin_names);
            add_token_with_spaces(admin_names.admin1_name, i);
            add_token_with_spaces(admin_names.admin2_name1, i);
            add_token_with_spaces(admin_names.admin2_name2, i);
        }
    }
}

int tokens_num() {
    return hash_map_size(tokens);
}

int has_token(char const *token) {
    return hash_map_get(tokens, token) != 0;
}

geoname_indices_t geonames_by_token(char const *token) {
    if (!has_token(token))
        return 0;

    return *((geoname_indices_t *) hash_map_get(tokens, token));
}

void dump_tokens(FILE *f) {
    int i, size = hash_map_capacity(tokens);
    int str_offset = 1, indices_offset = 0;

    fwrite(&size, sizeof size, 1, f);

    for (i = 0; i != size; ++i) {
        char const *token = hash_map_key(tokens, i);

        if (token) {
            int size = vector_size(geonames_by_token(token));

            fwrite(&str_offset, sizeof str_offset, 1, f);
            str_offset += strlen(token) + 1;
            
            fwrite(&indices_offset, sizeof indices_offset, 1, f);            
            indices_offset += sizeof(int) + (size > 1 ? size * sizeof(geoname_idx_t) : 0);
        } else {
            int data[] = {0, 0};
            fwrite(&data, sizeof data, 1, f);
        }               
    }

    {
        char zero = 0;
        fwrite(&str_offset, sizeof str_offset, 1, f);
        fwrite(&zero, sizeof zero, 1, f);
    }

    for (i = 0; i != size; ++i) {
        char const *token = hash_map_key(tokens, i);
        if (token)
            fwrite(token, 1, strlen(token) + 1, f);
    }

    for (i = 0; i != size; ++i) {
        if (hash_map_key(tokens, i)) {
            geoname_indices_t v = *((geoname_indices_t *) hash_map_value(tokens, i));
            int size = vector_size(v);

            if (size > 1) {
                int t = ~size;                    
                fwrite(&t, sizeof size, 1, f);
                fwrite(vector_at(v, 0), sizeof(geoname_idx_t), size, f);
            } else 
                fwrite(vector_at(v, 0), sizeof(geoname_idx_t), 1, f);
        }
    }
}