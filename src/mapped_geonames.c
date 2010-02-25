#include "standard.h"
#include "mapped_geonames.h"
#include "log.h"

#pragma pack(push, 1)

typedef struct {
    int id;
    int str_offset;
} mapped_geoname_t;

typedef struct {
    int                         num;
    mapped_geoname_t const *    geonames;
    int                         names_len;
    char const *                names;
} mapped_geonames_t;

#pragma pack(pop)

static mapped_geonames_t geonames;

void const * init_mapped_geonames(void const *p) {
    char const *ptr = (char const *) p;

    geonames.num = *(int *) ptr;
    ptr += sizeof geonames.num;
    debug("%d geonames\n", geonames.num);

    geonames.geonames = (mapped_geoname_t const *) ptr;
    ptr += sizeof(mapped_geoname_t) * geonames.num;
    debug("%dMB for geoname data\n", (sizeof(mapped_geoname_t) * geonames.num) >> 20);

    geonames.names_len = *(int *) ptr;
    ptr += sizeof geonames.names_len;

    geonames.names = (char const *) ptr;
    ptr += geonames.names_len;
    debug("%dMB for geoname strings\n", geonames.names_len >> 20);

    return ptr;
}

int mapped_geoname_id(int idx) {
    return geonames.geonames[idx].id;
}

char const * mapped_geoname_name(int idx) {
    return geonames.names + geonames.geonames[idx].str_offset;
}