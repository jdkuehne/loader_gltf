#ifndef JK_C_ALLOC_H
#define JK_C_ALLOC_H

#include "../core.hpp"

void *cstd_malloc(U64 size);
void cstd_free(void *memory);

// jdk: for allocator interface
U8 *cstd_malloc_wrapper(void *allocator_data, U64 size, U64 alignment);
void cstd_free_wrapper(void *allocator_data, void *memory);

#endif
