#include "allocator.hpp"

namespace base
{

U8 *malloc_wrapper(void *allocator_data, U64 size, U64 alignment) {
    return (U8 *)malloc(size);
}

void free_wrapper(void *allocator_data, void *memory) {
    free(memory);
}

}
