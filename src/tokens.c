#include "standard.h"
#include "tokens.h"
#include "geonames.h"
#include "hash_map.h"
#include "log.h"
#include "util.h"
#include "admin_codes.h"

static hash_map_t tokens;

typedef struct {
    int str_offset;
    int indices;
} token_info_t;

static vector_t tokens_str;
static vector_t geoname_indices;
static vector_t geoname_next;

static int add_token_text(char *str) {
    int i, len = strlen(str);

    for (i = 0; i <= len; ++i)
        vector_push(tokens_str, &str[i]);

    return vector_size(tokens_str) - 2;
}

static void add_token(char *str, geoname_idx_t geo_idx) {
    char *word = xstrdup(str);
    int len = strlen(word);
    int token_str_pos = -1;

    do {
        token_info_t * info = (token_info_t *) hash_map_get(tokens, word);

        if (!info) {
            token_info_t new_info;

            if (token_str_pos == -1)
                token_str_pos = add_token_text(str);

            new_info.str_offset = token_str_pos;
            new_info.indices = -1;

            info = (token_info_t *) hash_map_put(tokens, word, &new_info);
        }
        
        if (info->indices == -1 || geoname_idx(geoname_indices, info->indices) != geo_idx) {
            vector_push(geoname_indices, &geo_idx);
            vector_push(geoname_next, &info->indices);
            info->indices = vector_size(geoname_indices) - 1;
        }

        word[--len] = 0;
        --token_str_pos;
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
    char zero = 0;

    tokens = hash_map_init(sizeof(token_info_t));
    tokens_str = vector_init(sizeof(char));
    vector_push(tokens_str, &zero);

    geoname_indices = vector_init(sizeof(geoname_idx_t));
    geoname_next = vector_init(sizeof(int));

    for (i = geonames_num() - 1; i >= 0; --i) {
        geoname_t const *g = geoname(i);
        country_info_t const *ci = country(g->country_idx);

        if (!(i % 1000))
            debug("processing geoname %d, %d tokens, %d indices so far\n", geonames_num() - i - 1,
                  hash_map_size(tokens), vector_size(geoname_indices));

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

geoname_indices_t geonames_by_token_info(token_info_t const *info) {
    static vector_t tmp;
    int idx;

    if (tmp)
        vector_free(tmp);

    tmp = vector_init(sizeof(geoname_idx_t));
    for (idx = info->indices; idx != -1; idx = geoname_idx(geoname_next, idx)) {
        geoname_idx_t geoname = geoname_idx(geoname_indices, idx);
        vector_push(tmp, &geoname);
    }

    return tmp;
}

geoname_indices_t geonames_by_token(char const *token) {
    token_info_t const *info;

    if (!has_token(token))
        return 0;
    
    info = (token_info_t const *) hash_map_get(tokens, token);
    return geonames_by_token_info(info);
}

void dump_tokens(FILE *f) {
    int i, size = hash_map_capacity(tokens);
    int indices_offset = 0;

    fwrite(&size, sizeof size, 1, f);

    for (i = 0; i != size; ++i) {
        char const *token = hash_map_key(tokens, i);

        if (token) {
            token_info_t const * info = (token_info_t const *) hash_map_get(tokens, token);
            int size = vector_size(geonames_by_token_info(info));

            fwrite(&info->str_offset, sizeof info->str_offset, 1, f);
            fwrite(&indices_offset, sizeof indices_offset, 1, f);
            indices_offset += sizeof(int) + (size > 1 ? size * sizeof(geoname_idx_t) : 0);
        } else {
            int data[] = {0, 0};
            fwrite(&data, sizeof data, 1, f);
        }
    }

    {
        int size = vector_size(tokens_str);
        fwrite(&size, sizeof size, 1, f);
        fwrite(vector_at(tokens_str, 0), sizeof(char), size, f);
    }

    for (i = 0; i != size; ++i) {
        if (hash_map_key(tokens, i)) {
            geoname_indices_t v = geonames_by_token_info((token_info_t *) hash_map_value(tokens, i));
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
