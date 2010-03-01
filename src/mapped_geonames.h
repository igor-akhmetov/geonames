#pragma once

/* Implementation of a list of geonames over a memory-mapped file. */

void const *    init_mapped_geonames(void const *ptr);
int             mapped_geoname_id(int idx);
char const *    mapped_geoname_name(int idx);
