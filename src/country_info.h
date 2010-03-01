#pragma once

/* Countries database. */

typedef struct {
    char const *name;
    char const *iso;
    char const *iso3;
    char const *fips;
} country_info_t;

void                    load_countries(char const *file_name);
int                     countries_num();
country_info_t const *  country(int idx);
int                     country_idx_by_iso(char const *iso);
