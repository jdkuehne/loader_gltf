#include "c_alloc.hpp"
// @NOTE(jdk): not in header so stdlib won't be in global namespace
#include <stdlib.h>

void *cstd_malloc(U64 size) { return malloc(size); }
void cstd_free(void *memory) { free(memory); }

U8 *cstd_malloc_wrapper(void *allocator_data, U64 size, U64 alignment) {
    return (U8 *)cstd_malloc(size);
}

void cstd_free_wrapper(void *allocator_data, void *memory) {
    cstd_free(memory);
}
