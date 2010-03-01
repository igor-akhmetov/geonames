#include "standard.h"
#include "hash_map.h"
#include "vector.h"
#include "util.h"
#include "log.h"

struct hash_map_impl {
    int         elem_size; /* size of a single value */
    int         nentries;  /* number of entries */
    vector_t    keys;      /* keys of the hash map */
    vector_t    entries;   /* values of the hash map */
};

int hash_map_size(hash_map_t m) {
    return m->nentries;
}

int hash_map_capacity(hash_map_t m) {
    return vector_size(m->keys);
}

char * hash_map_key(hash_map_t m, int idx) {
    return *((char **) vector_at(m->keys, idx));
}

void * hash_map_value(hash_map_t m, int idx) {
    return vector_at(m->entries, idx);
}

hash_map_t hash_map_init(int elem_size)
{
    hash_map_t m = xcalloc(sizeof(struct hash_map_impl));
    m->elem_size = elem_size;
    m->keys = vector_init(sizeof(char *));
    m->entries = vector_init(elem_size);
    return m;
}

void hash_map_free(hash_map_t m) {
    int i, n;

    for (i = 0, n = vector_size(m->keys); i != n; ++i)
        free(hash_map_key(m, i));

    vector_free(m->keys);
    vector_free(m->entries);

    free(m);
}

void * hash_map_get(hash_map_t m, char const *key)
{
    int start_pos = 0, i = 0, j = 0, mask = 0;

    assert(m);
    assert(key);

    if (!m->nentries)
        return 0;

    mask = vector_size(m->keys) - 1;
    start_pos = strhash(key) & mask;

    for (i = start_pos;;) {
        char const *cur_key = hash_map_key(m, i);
        if (cur_key) {
            if (!strcmp(key, cur_key))
                return hash_map_value(m, i);
        } else
            break;

        i = (i + (j++ << 1) + 1) & mask;
        if (i == start_pos)
            break;
    }

    return NULL;
}

static void increase_hash_size(hash_map_t m) {
    hash_map_t new_map = hash_map_init(m->elem_size);
    vector_t tmp;
    int i = 0, cur_size = 0, new_size = 0;

    assert(m);

    cur_size = vector_size(m->keys);
    if (!cur_size)
        new_size = 4;
    else
        new_size = cur_size << 1;

    vector_resize(new_map->keys, new_size);
    vector_resize(new_map->entries, new_size);

    for (; i < cur_size; ++i) {
        char const *key = 0;
        if ((key = hash_map_key(m, i)))
            hash_map_put(new_map, key, hash_map_value(m, i));
    }

    tmp = m->keys; m->keys = new_map->keys; new_map->keys = tmp;
    tmp = m->entries; m->entries = new_map->entries; new_map->entries = tmp;

    hash_map_free(new_map);
}

void * hash_map_put(hash_map_t m, char const *key, void *data)
{
    int start_pos = 0, i = 0, j = 0, mask = 0;

    assert(m);
    assert(key);

    if ((m->nentries << 1) >= vector_size(m->keys))
        increase_hash_size(m);

    mask = vector_size(m->keys) - 1;
    start_pos = strhash(key) & mask;

    for (i = start_pos;;) {
        if (!hash_map_key(m, i)) {
            char * key_copy = xstrdup(key);
            ++m->nentries;
            vector_set(m->keys, i, &key_copy);
            return vector_set(m->entries, i, data);
        }

        i = (i + (j++ << 1) + 1) & mask;
        if (i == start_pos)
            error("hash wraparound");
    }
}
