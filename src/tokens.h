#pragma once

#include "vector.h"
#include "geonames.h"

void                    collect_tokens();
int                     tokens_num();
int                     has_token(char const *token);
void                    dump_tokens(FILE *f);
geoname_indices_t       geonames_by_token(char const *token);

typedef geoname_indices_t (*geonames_by_token_func)(char const *token);
geoname_indices_t       process_query(vector_t tokens, int max_results, geonames_by_token_func func);