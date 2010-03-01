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
static geoname_idx_t *geoname_indices;
static vector_t last_geoname_idx;
static vector_t token_indices_num;
static int *token_offset;
static int nindices, ntokens;
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
            vector_push(token_indices_num, &z);
        }

        if (*(int*)vector_at(last_geoname_idx, *pidx) > geo_idx) {
            int nused = (*(int *) vector_at(token_indices_num, *pidx)) + 1;
            vector_set(last_geoname_idx, *pidx, &geo_idx);
            vector_set(token_indices_num, *pidx, &nused);
            fwrite(pidx, sizeof(geoname_idx_t), 1, tmp_file);
            ++nindices;
        }

        word[--len] = 0;
        --token_str_pos;
    } while (len);
}

static void add_token_with_dashes(char *word, geoname_idx_t geo_idx) {
    vector_t parts;
    int i, n;
    char *copy;

    if (!word)
        return;

    copy = xstrdup(word);
    add_token(copy, geo_idx);
    free(copy);

    parts = strsplit(word, "-");

    for (i = 0, n = vector_size(parts); i != n; ++i)
        add_token(*(char **) vector_at(parts, i), geo_idx);

    vector_free(parts);
}

static void add_tokens(char const *str, geoname_idx_t geo_idx) {
    int j, n;
    char *token;
    vector_t words;

    if (!str)
        return;

    token = strlower(xstrdup(str));
    words = strsplit(token, " \t,");

    for (j = 0, n = vector_size(words); j != vector_size(words); ++j)
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

    ntokens = vector_size(token_indices_num);
}

static void calc_offsets() {
    int i;

    token_offset = xmalloc(ntokens * sizeof(int));
    token_offset[0] = *(int *) vector_at(token_indices_num, 0);
    for (i = 1; i < ntokens; ++i)
        token_offset[i] = token_offset[i - 1] + *(int *) vector_at(token_indices_num, i);
}

static void fill_indices() {
    int i;
    geoname_idx_t cur_geoname = geonames_num() - 1;

    geoname_indices = xmalloc(nindices * sizeof(geoname_idx_t));

    for (i = 0; i < nindices + geonames_num(); ++i) {
        int token;
        fread(&token, sizeof token, 1, tmp_file);
        if (token == -1)
            --cur_geoname;
        else {
            --token_offset[token];
            geoname_indices[token_offset[token]] = cur_geoname;
        }
    }
}

static unsigned calc_hash(int *p, int *q, int hash_size) {
    unsigned res = 0;
    while (p != q)
        res = (res * 37 + *p++ + 1) % hash_size;
    return res;
}

static int are_indices_equal(int t1, int t2) {    
    int *p1 = geoname_indices + token_offset[t1];
    int *q1 = p1 + *(int *) vector_at(token_indices_num, t1);

    int *p2 = geoname_indices + token_offset[t2];
    int *q2 = p2 + *(int *) vector_at(token_indices_num, t2);

    if (q1 - p1 != q2 - p2)
        return 0;

    for (; p1 != q1; ++p1, ++p2)
        if (*p1 != *p2)
            return 0;

    return 1;
}

void compress_indices() {
    int i, hash_size = (ntokens << 1) + 1;
    int *hash_to_token = xmalloc(hash_size * sizeof(int));
    memset(hash_to_token, -1, hash_size * sizeof(int));

    debug("compressing indices...\n");

    nindices = 0;
    for (i = 0; i < ntokens; ++i) {
        int offset = token_offset[i];
        int len = *(int *) vector_at(token_indices_num, i);
        int next_offset = offset + len;
        int hash_val = calc_hash(geoname_indices + offset,
                                 geoname_indices + next_offset,
                                 hash_size);
        int prev = hash_to_token[hash_val];

        if (prev != -1 && are_indices_equal(prev, i))
            token_offset[i] = token_offset[prev];            
        else {
            hash_to_token[hash_val] = i;            
            token_offset[i] = nindices;
            memmove(geoname_indices + nindices, geoname_indices + offset, len * sizeof(int));
            nindices += len;
        }
    }

    debug("%d indices after compression\n", nindices);
    free(hash_to_token);
}

void process_tokens() {
    char zero = 0;

    tokens = hash_map_init(sizeof(int));
    tokens_str = vector_init(sizeof(char));
    str_offset = vector_init(sizeof(int));
    last_geoname_idx = vector_init(sizeof(int));
    token_indices_num = vector_init(sizeof(int));

    tmp_file = tmpfile();
    if (!tmp_file)
        cerror("can't create temporary file");

    /* Need NULL character as a first element of the names
       string in order to share one name between several tokens. */
    vector_push(tokens_str, &zero);

    collect_tokens();
    debug("%d tokens, %d indices\n", ntokens, nindices);
    calc_offsets();
    rewind(tmp_file);
    fill_indices();

    if (fclose(tmp_file) == EOF)
        cerror("can't close temporary file");

    compress_indices();
}

int has_token(char const *token) {
    return hash_map_get(tokens, token) != 0;
}

geoname_indices_t geonames_by_token(char const *token) {
    int *pidx = (int *) hash_map_get(tokens, token);
    if (!pidx)
        return 0;

    {
        int cur_offset = token_offset[*pidx];
        int len = *(int *) vector_at(token_indices_num, *pidx);

        return vector_from_memory(sizeof(geoname_idx_t),
                                  len, geoname_indices + cur_offset);
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
    int size;

    fwrite(&ntokens, sizeof ntokens, 1, f);
    fwrite(vector_at(str_offset, 0), sizeof(int), ntokens, f);
    fwrite(token_offset, sizeof(int), ntokens, f);
    fwrite(vector_at(token_indices_num, 0), sizeof(int), ntokens, f);

    size = vector_size(tokens_str);
    fwrite(&size, sizeof size, 1, f);
    fwrite(vector_at(tokens_str, 0), sizeof(char), size, f);
    
    fwrite(&nindices, sizeof nindices, 1, f);    
    fwrite(geoname_indices, sizeof(geoname_idx_t), nindices, f);
}

void dump_tokens(FILE *f) {
    dump_hashmap(f);
    dump_tokens_data(f);
}
