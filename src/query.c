#include "standard.h"
#include "util.h"
#include "vector.h"
#include "mapped_geonames.h"
#include "mapped_tokens.h"
#include "log.h"
#include "process_query.h"

static int const MAX_RESULTS = 10;

static void load_data(char const *filename) {
    void const *data = map_file_read(filename);
    data = init_mapped_geonames(data);
    init_mapped_tokens(data);
}

static void print_geoname_info(geoname_idx_t idx) {
    printf("%d\t%s\n", mapped_geoname_id(idx), mapped_geoname_name(idx));
}

int main(int argc, char *argv[]) {    
    if (argc != 2)
        error("usage: %s [data file]\n", argv[0]);

    load_data(argv[1]);
    run_interactive_loop(mapped_geonames_by_token, MAX_RESULTS, print_geoname_info);

    return EXIT_SUCCESS;
}