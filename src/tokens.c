#include "standard.h"
#include "tokens.h"
#include "geonames.h"
#include "hash_map.h"
#include "log.h"
#include "util.h"
#include "admin_codes.h"

static hash_map_t tokens;
static vector_t tokens_str;
static vector_t str_offset;
static vector_t geoname_indices;
static vector_t last_geoname_idx;
static vector_t token_offset;
static int nindices;
static FILE *tmp_file;

static int add_token_text(char *str) {
    int i, len = strlen(str);

    for (i = 0; i <= len; ++i)
        vector_push(tokens_str, &str[i]);

    return vector_size(tokens_str) - 2;
}

static void add_token(char *word, geoname_idx_t geo_idx) {
    int len = strlen(word);
    int token_str_pos = -1;

    do {
        int *pidx = (int *) hash_map_get(tokens, word);

        if (!pidx) {
            int z = 0, inf = 1 << 30, idx = vector_size(str_offset);
            pidx = (int *) hash_map_put(tokens, word, &idx);

            if (token_str_pos == -1)
                token_str_pos = add_token_text(word);

            vector_push(str_offset, &token_str_pos);
            vector_push(last_geoname_idx, &inf);
            vector_push(token_offset, &z);
        }

        if (*(int*)vector_at(last_geoname_idx, *pidx) > geo_idx) {
            int nused = (*(int *) vector_at(token_offset, *pidx)) + 1;
            vector_set(last_geoname_idx, *pidx, &geo_idx);
            vector_set(token_offset, *pidx, &nused);
            fwrite(pidx, sizeof(geoname_idx_t), 1, tmp_file);
            ++nindices;
        }

        word[--len] = 0;
        --token_str_pos;
    } while (len);
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

static void add_tokens(char const *str, geoname_idx_t geo_idx) {
    int j;
    char *token;
    vector_t words;

    if (!str)
        return;

    token = strlower(xstrdup(str));
    words = strsplit(token, " \t,");

    for (j = 0; j != vector_size(words); ++j)
        add_token_with_dashes(*(char **)vector_at(words, j), geo_idx);

    vector_free(words);
    free(token);
}

static void collect_tokens() {
    int i;

    for (i = geonames_num() - 1; i >= 0; --i) {
        geoname_t const *g = geoname(i);
        country_info_t const *ci = country(g->country_idx);

        if (!(i % 1000))
            debug("processing geoname %d, %d tokens so far\n",
                  geonames_num() - i, vector_size(str_offset));

        add_tokens(g->name, i);
        add_tokens(g->alternate_names, i);
        add_tokens(g->admin_names.admin1_name, i);
        add_tokens(g->admin_names.admin2_name1, i);
        add_tokens(g->admin_names.admin2_name2, i);

        if (ci) {
            add_tokens(ci->name, i);
            add_tokens(ci->fips, i);
            add_tokens(ci->iso, i);
            add_tokens(ci->iso3, i);
        }
        {
            int bound = -1;
            fwrite(&bound, sizeof bound, 1, tmp_file);
        }
    }

    vector_free(last_geoname_idx);
}

static void calc_offsets() {
    int i, ntokens = vector_size(token_offset);

    for (i = 1; i < ntokens; ++i) {
        int sum = *(int *) vector_at(token_offset, i) +
                  *(int *) vector_at(token_offset, i - 1);
        vector_set(token_offset, i, &sum);
    }

    vector_push(token_offset, &nindices);
}

static void fill_indices() {
    int i;
    geoname_idx_t cur_geoname = geonames_num() - 1;

    geoname_indices = vector_init(sizeof(geoname_idx_t));
    vector_resize(geoname_indices, nindices);

    for (i = 0; i < nindices + geonames_num(); ++i) {
        int token;
        fread(&token, sizeof token, 1, tmp_file);
        if (token == -1)
            --cur_geoname;
        else {
            int pos = *(int *) vector_at(token_offset, token);
            --pos;
            vector_set(geoname_indices, pos, &cur_geoname);
            vector_set(token_offset, token, &pos);
        }
    }
}

void process_tokens() {
    tokens = hash_map_init(sizeof(int));
    tokens_str = vector_init(sizeof(char));
    str_offset = vector_init(sizeof(int));
    last_geoname_idx = vector_init(sizeof(int));
    token_offset = vector_init(sizeof(int));

    tmp_file = tmpfile();
    if (!tmp_file)
        cerror("can't create temporary file");

    collect_tokens();
    debug("%d tokens, %d indices\n", vector_size(token_offset), nindices);
    calc_offsets();
    rewind(tmp_file);
    fill_indices();

    if (fclose(tmp_file) == EOF)
        cerror("can't close temporary file");
}

int has_token(char const *token) {
    return hash_map_get(tokens, token) != 0;
}

geoname_indices_t geonames_by_token(char const *token) {
    int *pidx = (int *) hash_map_get(tokens, token);
    if (!pidx)
        return 0;

    {
        int cur_offset = *(int *) vector_at(token_offset, *pidx);
        int next_offset = *(int *) vector_at(token_offset, *pidx + 1);

        return vector_from_memory(sizeof(geoname_idx_t),
                                  next_offset - cur_offset,
                                  vector_at(geoname_indices, cur_offset));
    }
}

static void dump_hashmap(FILE *f) {
    int i, size = hash_map_capacity(tokens);

    fwrite(&size, sizeof size, 1, f);

    for (i = 0; i != size; ++i) {
        char const *token = hash_map_key(tokens, i);

        if (token)
            fwrite(hash_map_value(tokens, i), sizeof(int), 1, f);
        else {
            int neg = -1;
            fwrite(&neg, sizeof neg, 1, f);
        }
    }
}

static void dump_tokens_data(FILE *f) {
    int size, ntokens = vector_size(str_offset);

    fwrite(&ntokens, sizeof ntokens, 1, f);
    fwrite(vector_at(str_offset, 0), sizeof(int), ntokens + 1, f);
    fwrite(vector_at(token_offset, 0), sizeof(int), ntokens + 1, f);

    size = vector_size(tokens_str);
    fwrite(&size, sizeof size, 1, f);
    fwrite(vector_at(tokens_str, 0), sizeof(char), size, f);

    size = vector_size(geoname_indices);
    fwrite(&size, sizeof size, 1, f);
    fwrite(vector_at(geoname_indices, 0), sizeof(geoname_idx_t), nindices, f);
}

void dump_tokens(FILE *f) {
    dump_hashmap(f);
    dump_tokens_data(f);
}
