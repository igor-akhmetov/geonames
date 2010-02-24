#pragma once

#include "vector.h"
#include "geonames.h"
#include "process_query.h"

void                    collect_tokens();
int                     tokens_num();
int                     has_token(char const *token);
void                    dump_tokens(FILE *f);
geoname_indices_t       geonames_by_token(char const *token);
