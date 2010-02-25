#include "standard.h"
#include "geonames.h"
#include "country_info.h"
#include "log.h"
#include "text_db.h"
#include "util.h"

enum {
    ID_FIELD = 0,
    NAME_FIELD = 1,
    ALTERNATE_NAMES_FIELD = 3,
    COUNTRY_FIELD = 8,
    ADMIN1_CODE_FIELD = 10,
    ADMIN2_CODE_FIELD = 11,
    POPULATION_FIELD = 14
};

static vector_t geonames;

void load_geonames(char const * filename) {
    text_db_t db = tdb_open(filename);

    geonames = vector_init(sizeof(geoname_t));
    
    while (tdb_next_row(db)) {
        geoname_t g = {0};        
        char const *admin1_code;
        char const *admin2_code;

        char const * cat = tdb_field(db, 6);
        if (strcmp(cat, "P"))
            continue;

        g.id                = atoi(tdb_field(db, ID_FIELD));
        g.name              = xstrdup(tdb_field(db, NAME_FIELD));
        g.alternate_names   = xstrdup(tdb_field(db, ALTERNATE_NAMES_FIELD));
        g.country_idx       = country_idx_by_iso(tdb_field(db, COUNTRY_FIELD));
        g.population        = atoi(tdb_field(db, POPULATION_FIELD));

        if (g.country_idx != -1) {
            admin1_code         = tdb_field(db, ADMIN1_CODE_FIELD);
            admin2_code         = tdb_field(db, ADMIN2_CODE_FIELD);

            get_admin_names(g.country_idx, admin1_code, admin2_code, &g.admin_names);
        }

        vector_push(geonames, &g);
    }

    tdb_close(db);
}

int geoname_compare(void const *p, void const *q) {
    geoname_t const *x = p;
    geoname_t const *y = q;

    return y->population - x->population;
}

void sort_geonames() {
    vector_sort(geonames, geoname_compare);
}

int geonames_num() {
    return vector_size(geonames);
}

geoname_t const *geoname(int idx) {    
    return (geoname_t const *) vector_at(geonames, idx);
}

void dump_geonames_text() {
    int i = 0, n = geonames_num();

    printf("All geonames (id, name, country id, population):\n");
    for (i = 0; i != n; ++i) {
        geoname_t const *g = geoname(i);
        printf("%d\t%s\t%d\t%d\n", g->id, g->name, g->country_idx, g->population);
    }
}

void dump_geonames(FILE *f) {
    int i, offset = 0, num = geonames_num();

    fwrite(&num, sizeof num, 1, f);

    for (i = 0; i != geonames_num(); ++i) {
        geoname_t const *g = geoname(i);

        fwrite(&g->id, sizeof g->id, 1, f);
        fwrite(&offset, sizeof offset, 1, f);

        offset += strlen(geoname(i)->name) + 1;
    }

    fwrite(&offset, sizeof offset, 1, f);

    for (i = 0; i != geonames_num(); ++i)
        fwrite(geoname(i)->name, strlen(geoname(i)->name) + 1, 1, f);
}
