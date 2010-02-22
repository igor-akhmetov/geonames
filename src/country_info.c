#include "standard.h"
#include "country_info.h"
#include "vector.h"
#include "log.h"
#include "text_db.h"
#include "hash_map.h"
#include "util.h"

static vector_t     countries;
static hash_map_t   idx_by_iso;

enum {
    ISO_FIELD = 0,
    ISO3_FIELD = 1,
    FIPS_FIELD = 3,
    NAME_FIELD = 4
};

void load_countries(char const * filename)
{
    text_db_t db = tdb_open(filename);

    countries = vector_init(sizeof(country_info_t));
    idx_by_iso = hash_map_init(sizeof(int));

    while (tdb_next_row(db)) {        
        country_info_t c = {0};
        int idx = 0;

        c.iso   = strdup(tdb_field(db, ISO_FIELD));
        c.iso3  = strdup(tdb_field(db, ISO3_FIELD));
        c.fips  = strdup(tdb_field(db, FIPS_FIELD));
        c.name  = strdup(tdb_field(db, NAME_FIELD));

        idx = vector_size(countries);
        hash_map_put(idx_by_iso, c.iso, &idx);
        
        vector_push(countries, &c);
    }

    tdb_close(db);
}

int countries_num() {
    return vector_size(countries);
}

country_info_t const *country(int idx) {
    if (idx < 0 || idx >= vector_size(countries))
        return 0;
    return (country_info_t const *) vector_at(countries, idx);
}

int country_idx_by_iso(char const *iso) {
    int *p = hash_map_get(idx_by_iso, iso);
    return p ? *p : -1;
}

void dump_countries() {
    int i = 0, n = countries_num();

    printf("All countries (name, iso, iso3, fips):\n");
    for (i = 0; i != n; ++i) {
        country_info_t const *c = country(i);
        printf("%s\t%s\t%s\t%s\n", c->name, c->iso, c->iso3, c->fips);
    }
}