#pragma once

#include "vector.h"
#include "country_info.h"
#include "admin_codes.h"

typedef struct {
    int             id;
    char const      *name;
    char const      *alternate_names;
    int             country_idx;
    int             population;
    admin_names_t   admin_names;
} geoname_t;

void                load_geonames(char const *filename);
void                sort_geonames();
int                 geonames_num();
geoname_t const *   geoname(int idx);
void                dump_geonames_text();
void                dump_geonames(FILE *f);
