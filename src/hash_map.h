#pragma once

struct hash_map_impl;
typedef struct hash_map_impl * hash_map_t;

hash_map_t      hash_map_init(int elem_size);
void            hash_map_free(hash_map_t m);
void *          hash_map_get(hash_map_t m, char const *key);
void *          hash_map_put(hash_map_t m, char const *key, void *data);
int             hash_map_size(hash_map_t m);
int             hash_map_capacity(hash_map_t m);
char *          hash_map_key(hash_map_t m, int idx);
void *          hash_map_value(hash_map_t m, int idx);