/* Run interactive queries on a preprocessed database of geonames. */

#include "standard.h"
#include "util.h"
#include "vector.h"
#include "mapped_geonames.h"
#include "mapped_tokens.h"
#include "log.h"
#include "process_query.h"

static int const MAX_RESULTS = 10; /* print no more than this number of results */

static char const *dump_filename;  /* name of the file with the database of geonames */
static int use_mmap;               /* should we mmap the data file instead of reading it? */
static int populate_data;          /* should we ask the OS to populate mmaped file? */

/* Parse command-line arguments. */
static void parse_args(int argc, char *argv[]) {
    set_program_name(argv[0]);
    --argc; ++argv;

    while (argc) {
        if (!strcmp(argv[0], "-v"))
            set_verbose(1);
        else if (!strcmp(argv[0], "-m"))
            use_mmap = 1;
        else if (!strcmp(argv[0], "-p"))
            populate_data = 1;
        else
            break;

        --argc; ++argv;
    }

    if (argc != 1)
        usage("[-p] [-v] [data file]\nArguments:\n"
              "    -m  use mmap for reading the data file,\n"
              "        instant start up time but slower queries\n"
              "    -p  populate mapped memory when using -m,\n"
              "        make queries faster but use more memory\n"
              "    -v  enable debug output");

    dump_filename = argv[0];
}

static void load_data(char const *filename) {
    void const *data;

    if (use_mmap)
        data = map_file_read(filename, populate_data);
    else
        data = read_file(filename);

    data = init_mapped_geonames(data);
    init_mapped_tokens(data);
}

static void print_geoname_info(geoname_idx_t idx) {
    printf("%d\t%s\n", mapped_geoname_id(idx.idx), mapped_geoname_name(idx.idx));
}

int main(int argc, char *argv[]) {
    parse_args(argc, argv);
    load_data(dump_filename);
    run_interactive_loop(mapped_geonames_by_token, MAX_RESULTS, print_geoname_info);
    return EXIT_SUCCESS;
}
