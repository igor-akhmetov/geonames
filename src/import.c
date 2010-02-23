#include "standard.h"
#include "country_info.h"
#include "geonames.h"
#include "log.h"
#include "tokens.h"
#include "util.h"
#include "admin_codes.h"

static char const *COUNTRY_INFO_FILE    = "countryInfo.txt";
static char const *GEONAMES_INFO_FILE   = "allCountries.txt";
static char const *ADMIN1_CODES_FILE    = "admin1Codes.txt";
static char const *ADMIN2_CODES_FILE    = "admin2Codes.txt";

static void load_data() {
    debug("reading country info...\n");
    load_countries(COUNTRY_INFO_FILE);
    debug("done, %d countries total\n", countries_num());

     debug("reading admin codes...\n");
    load_admin_codes(ADMIN1_CODES_FILE, ADMIN2_CODES_FILE);
    debug("done\n", countries_num());

    debug("reading geonames...\n");
    load_geonames(GEONAMES_INFO_FILE);
    debug("done, %d geonames total\n", geonames_num());
}

static void process() {    
    debug("sorting geonames by population...\n");
    sort_geonames();
    debug("done, %d geonames total\n", geonames_num());

    debug("building table of tokens...\n");
    collect_tokens();
    debug("done, %d tokens total\n", tokens_num());
}

static void dump_data(char const *filename) {
    FILE *f= fopen("dump", "wb");
    if (!f)
        cerror("can't open file %s for writing", filename);

    debug("saving geonames...\n");
    dump_geonames(f);
    debug("done\n");

    debug("saving tokens table...\n");
    dump_tokens(f);
    debug("done\n");
}

static void serve_queries() {
    char q[1024];        

    for (;;) {
        int i;
        vector_t tokens;
        geoname_indices_t geonames;

        printf("Query: ");
        if (!fgets(q, sizeof q, stdin))
            break;

        strlower(strtrim(q));

        if (!*q)
            continue;

        tokens = strsplit(q, " \t");
        geonames = process_query(tokens, 0, geonames_by_token);

        for (i = 0; i != vector_size(geonames); ++i) {
            geoname_t const *geo = geoname(geoname_idx(geonames, i));
            country_info_t const *ci = country(geo->country_idx);
            printf("%d. %d %s, %s, population %d\n", i + 1, geo->id, geo->name,
                ci ? ci->name : "unknown country", geo->population);
        }

        vector_free(tokens);
        vector_free(geonames);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2)
        error("usage: %s [data file]\n", argv[0]);

    load_data();
    process();
    dump_data(argv[1]);
    serve_queries();

    return EXIT_SUCCESS;
}