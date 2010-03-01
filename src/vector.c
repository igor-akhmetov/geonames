#include "standard.h"

#include "vector.h"
#include "util.h"

/* Initial number of elements in a vector. */
static const int VECTOR_INITIAL_SIZE = 4;

typedef struct vector_impl {
    int     elem_size;  /* size of an element */
    int     size;       /* total number of elements in the array */
    int     capacity;   /* allocated number of elements */
    void    *data;      /* pointer to reserved memory */
} vector_impl;

vector_t vector_init(int elem_size) {
    vector_t v = xcalloc(sizeof(vector_impl));
    v->elem_size = elem_size;
    return v;
}

vector_t vector_from_memory(int elem_size, int size, void const *ptr) {
    vector_t v = xcalloc(sizeof(vector_impl));
    v->elem_size = elem_size;
    v->size = size;
    v->capacity = size;
    v->data = (void *) ptr;
    return v;
}

void vector_free(vector_t v) {
    free(v->data);
    free(v);
}

void * vector_at(vector_t v, int idx) {
    return (char *) v->data + idx * v->elem_size;
}

int vector_size(vector_t v) {
    return v ? v->size : 0;
}

int vector_capacity(vector_t v) {
    return v ? v->capacity : 0;
}

void vector_reserve(vector_t v, int capacity) {
    v->capacity = capacity;
    v->data = xrealloc(v->data, v->capacity * v->elem_size);
}

void vector_resize(vector_t v, int new_size) {
    int cur_size = 0;

    assert(v);

    cur_size = vector_size(v);
    vector_reserve(v, new_size);

    v->size = new_size;
    if (cur_size < new_size)
        memset(vector_at(v, cur_size), 0, (new_size - cur_size) * v->elem_size);
}

void *vector_set(vector_t v, int idx, void *data) {
    return memcpy(vector_at(v, idx), data, v->elem_size);
}

void *vector_push(vector_t v, void *data) {
    assert(v);

    if (v->size == v->capacity) {
        if (!v->data)
            vector_reserve(v, VECTOR_INITIAL_SIZE);
        else
            vector_reserve(v, (int)(v->capacity * 1.5));
    }

    ++v->size;
    return memcpy(vector_at(v, v->size - 1), data, v->elem_size);
}

void vector_sort(vector_t v, vector_compare_func func) {
    qsort(vector_at(v, 0), vector_size(v), v->elem_size, func);
}
