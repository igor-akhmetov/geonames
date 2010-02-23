#include "standard.h"
#include "util.h"
#include "vector.h"
#include "mapped_geonames.h"
#include "mapped_tokens.h"
#include "log.h"

static int const MAX_RESULTS = 10;

static void load_data(char const *filename) {
    void const *data = map_file_read(filename);
    data = init_mapped_geonames(data);
    init_mapped_tokens(data);
}

static void serve_queries() {
    for (;;) {
        int i;
        char q[1024];

        vector_t tokens;
        geoname_indices_t res;

        if (!fgets(q, sizeof q, stdin))
            break;

        strlower(strtrim(q));

        if (!*q)
            continue;

        tokens = strsplit(q, " \t");
        res = process_query(tokens, MAX_RESULTS, mapped_geonames_by_token);

        for (i = 0; i != vector_size(res); ++i) {
            int idx = geoname_idx(res, i);
            printf("%d\t%s\n", mapped_geoname_id(idx), mapped_geoname_name(idx));
        }

        vector_free(tokens);
        vector_free(res);
    }
}

int main(int argc, char *argv[]) {    
    if (argc != 2)
        error("usage: %s [data file]\n", argv[0]);

    load_data(argv[1]);
    debug("Ready to serve\n");
    serve_queries();

    return EXIT_SUCCESS;
}