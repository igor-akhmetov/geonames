#pragma once

/* Implementation of a hash map from token names to
   lists of geoname indices over a memory-mapped file. */

#include "process_query.h"

/* Initialize the hash map. */
void                    init_mapped_tokens(void const *ptr);

/* Return a list of geoname indices for the given token. */
geoname_indices_t       mapped_geonames_by_token(char const *token);
