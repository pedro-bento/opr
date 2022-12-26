#ifndef DA_H
#define DA_H

#include <string.h>

#include "error.h"

#define __da_unpack(da) (void**)&(da)->items, sizeof(*(da)->items), &(da)->count, &(da)->capacity, (void*)&(da)->free_ptr

#define da_append(da,item) __da_append(__da_unpack(da),(void*)(item))

void __da_append(void **items, size_t item_size, size_t *count, size_t *capacity, void **free_ptr, void *item);

#endif // DA_H

#ifdef DA_IMPLEMENTATION

void __da_append(void **items, size_t item_size, size_t *count, size_t *capacity, void **free_ptr, void *item) {
    (void) free_ptr;

    if (*count >= *capacity) {
        *capacity = *capacity == 0 ? 1 : *capacity * 2;
        *items = realloc(*items, *capacity * item_size);
        error_assert(*items != NULL, "could not allocate %zu bytes of memory", *capacity * item_size);
        *free_ptr = *items;
    }
    
    size_t offset = *count * item_size;
    memcpy(*items + offset, item, item_size);
    *count += 1;
}

#endif // DA_IMPLEMENTATION