#include "standard.h"
#include "country_info.h"
#include "geonames.h"
#include "log.h"
#include "tokens.h"
#include "util.h"
#include "admin_codes.h"
#include "process_query.h"

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

static void print_geoname_info(geoname_idx_t geoname_idx) {
    geoname_t const *geo = geoname(geoname_idx);
    country_info_t const *ci = country(geo->country_idx);
    printf("%d %s, %s, population %d\n", geo->id, geo->name,
           ci ? ci->name : "unknown country", geo->population);
}

int main(int argc, char *argv[]) {
    if (argc != 2)
        error("usage: %s [data file]\n", argv[0]);

    load_data();
    process();
    dump_data(argv[1]);
    run_interactive_loop(geonames_by_token, 0, print_geoname_info);

    return EXIT_SUCCESS;
}