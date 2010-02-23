#pragma once

struct vector_impl;
typedef struct vector_impl * vector_t;

vector_t    vector_init(int elem_size);
vector_t    vector_from_memory(int elem_size, int size, void const *ptr);
void        vector_free(vector_t v);
void *      vector_at(vector_t v, int idx);
int         vector_size(vector_t v);
int         vector_capacity(vector_t v);
void        vector_reserve(vector_t v, int capacity);
void        vector_resize(vector_t v, int size);
void *      vector_set(vector_t v, int idx, void *data);
void *      vector_push(vector_t v, void *data);

typedef int (*vector_compare_func)(const void *, const void *);
void        vector_sort(vector_t v, vector_compare_func func);