#pragma once

/* Implementation of the common merge phase of the query algorithm,
   which can work with both in-memory and mapped data representations. */

#include "vector.h"

typedef struct {
  unsigned type : 1;
  unsigned idx  : 31;
} geoname_idx_t;

typedef vector_t    geoname_indices_t; /* vector of geoname indices */

typedef enum {
  GEONAME_PLACE = 1,
} GEONAME_TYPE;

/* Helper function to ease operations with vector. */
geoname_idx_t       geoname_idx(geoname_indices_t v, int idx);

/* A function which returns a vector of geoname indices
   for the given token. */
typedef geoname_indices_t (*geonames_by_token_func)(char const *token);

/* A callback for processing geonames in the vector of query results. */
typedef void              (*process_geoname_id_func)(geoname_idx_t geoname);

/* Run interactive loop, which queries for a string, parses it
   into tokens, generates a list of search results for this list
   of tokens and processes each result with the given function.
   Args:
     geonames_func - returns a list of matching geonames for a single token.
     max_results   - maximum number of results to process.
     process_func  - a callback for processing result geonames. */
void run_interactive_loop(geonames_by_token_func geonames_func,
                          int max_results,
                          process_geoname_id_func process_func);
