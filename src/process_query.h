#pragma once

#include "vector.h"

typedef int         geoname_idx_t;
typedef vector_t    geoname_indices_t;

geoname_idx_t       geoname_idx(geoname_indices_t v, int idx);

typedef geoname_indices_t (*geonames_by_token_func)(char const *token);

geoname_indices_t process_query(vector_t tokens, int max_results, geonames_by_token_func func);