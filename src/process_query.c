#include "standard.h"
#include "process_query.h"
#include "util.h"
#include "log.h"

geoname_idx_t geoname_idx(geoname_indices_t v, int idx) {
    return *(geoname_idx_t *) vector_at(v, idx);
}

geoname_indices_t process_query(vector_t tokens, int max_results, geonames_by_token_func func) {
    int i, ntokens = vector_size(tokens);

    geoname_indices_t   *lists = xmalloc(ntokens * sizeof(vector_t));
    int                 *sizes = xmalloc(ntokens * sizeof(int));
    int                 *pos   = xmalloc(ntokens * sizeof(int));

    geoname_indices_t res = vector_init(sizeof(geoname_idx_t));

    for (i = 0; i < ntokens; ++i) {
        char const *token = *(char const **) vector_at(tokens, i);

        lists[i] = func(token);
        sizes[i] = vector_size(lists[i]);
        pos[i]   = 0;

        if (!sizes[i])
            goto end;
    }

    for (;;) {
        int idx = 0;
        for (i = 0; i < ntokens; ++i) {
            if (pos[i] == sizes[i])
                goto end;
            if (geoname_idx(lists[i], pos[i]) < geoname_idx(lists[idx], pos[idx]))
                idx = i;
        }

        for (i = 0; i < ntokens; ++i) {
            if (geoname_idx(lists[i], pos[i]) != geoname_idx(lists[idx], pos[idx]))
                break;
        }

        if (i == ntokens) {
            vector_push(res, vector_at(lists[idx], pos[idx]));
            if (max_results == vector_size(res))
                goto end;
            for (i = 0; i < ntokens; ++i)
                ++pos[i];
        } else {
            ++pos[idx];
        }
    }

end:
    free(pos);
    free(sizes);
    free(lists);

    return res;
}

void run_interactive_loop(geonames_by_token_func geonames_func,
                          int max_results,
                          process_geoname_id_func process_func) {
    char q[4096];

    debug("Ready to serve\n");

    for (;;) {
        int i;
        vector_t tokens;
        geoname_indices_t geonames;

        if (!fgets(q, sizeof q, stdin))
            break;

        strlower(strtrim(q));

        if (!*q) {
            puts("");
            continue;
        }

        tokens = strsplit(q, " \t");
        geonames = process_query(tokens, max_results, geonames_func);

        for (i = 0; i != vector_size(geonames); ++i)
            process_func(geoname_idx(geonames, i));

        puts("");

        vector_free(tokens);
        vector_free(geonames);
    }
}
