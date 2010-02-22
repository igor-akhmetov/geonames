#pragma once

#include "tokens.h"

void                    init_mapped_tokens(void const *ptr);
geoname_indices_t       mapped_geonames_by_token(char const *token);