#pragma once

/* Token is a prefix of a string relevant to a geoname
   (name, country, etc). The purpose of this module is
   to processes the geonames database, extract all valid tokens
   and for each token save a list of indices of the geonames,
   which contain this token. */

#include "vector.h"
#include "geonames.h"
#include "process_query.h"

/* Generate tokens by processing all the geonames and
   construct necessary data for query resolution. */
void                    process_tokens();

/* Dump all necessary data to a file to be used later
   by a query program. */
void                    dump_tokens(FILE *f);

/* Return a list of geoname indices which match the given token. */
geoname_indices_t       geonames_by_token(char const *token);
