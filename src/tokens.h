#pragma once

#include "vector.h"
#include "geonames.h"
#include "process_query.h"

void                    process_tokens();
int                     has_token(char const *token);
void                    dump_tokens(FILE *f);
geoname_indices_t       geonames_by_token(char const *token);
