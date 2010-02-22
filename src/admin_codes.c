#include "standard.h"
#include "admin_codes.h"
#include "hash_map.h"
#include "text_db.h"
#include "country_info.h"

hash_map_t admin1_names;
hash_map_t admin2_names;

typedef struct {
    char const *name1;
    char const *name2;
} admin2_names_t;

enum {
    ADMIN1_NAME_FIELD = 1,
    ADMIN2_NAME1_FIELD = 1,
    ADMIN2_NAME2_FIELD = 2
};

void load_admin1_codes(char const *filename) {
    text_db_t db = tdb_open(filename);

    admin1_names = hash_map_init(sizeof(char const *));

    while (tdb_next_row(db)) {        
        char *name = strdup(tdb_field(db, 1));
        hash_map_put(admin1_names, tdb_field(db, ADMIN1_NAME_FIELD), &name);
    }

    tdb_close(db);
}

void load_admin2_codes(char const *filename) {
    text_db_t db = tdb_open(filename);

    admin2_names = hash_map_init(sizeof(admin2_names_t));

    while (tdb_next_row(db)) {        
        admin2_names_t names;

        names.name1 = strdup(tdb_field(db, ADMIN2_NAME1_FIELD));
        names.name2 = strdup(tdb_field(db, ADMIN2_NAME2_FIELD));

        hash_map_put(admin2_names, tdb_field(db, 0), &names);
    }

    tdb_close(db);
}

void load_admin_codes(char const *admin1_filename, char const *admin2_filename) {
    load_admin1_codes(admin1_filename);
    load_admin2_codes(admin2_filename);
}

void get_admin_names(int country_idx, char const *code1, char const *code2, admin_names_t * names) {
    char buf[64];
    country_info_t const *ci = country(country_idx);
    char const **admin1;
    admin2_names_t const *admin2;

    assert(names);
    assert(ci);

    memset(names, 0, sizeof(admin_names_t));

    if (!code1)
        return;

    snprintf(buf, sizeof buf, "%s.%s", ci->iso, code1);
    admin1 = (char const **) hash_map_get(admin1_names, buf);
    if (admin1)
        names->admin1_name = *admin1;

    if (!code2)
        return;

    snprintf(buf, sizeof buf, "%s.%s.%s", ci->iso, code1, code2);
    admin2 = (admin2_names_t const *) hash_map_get(admin2_names, buf);
    if (admin2) {
        names->admin2_name1 = admin2->name1;
        names->admin2_name2 = admin2->name2;
    }
}