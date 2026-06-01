#ifndef JK_ALLOCATOR_H
#define JK_ALLOCATOR_H

#include "arena.hpp"

namespace base::mem
{

// @TODO(jdk): maybe change so allocators have their data at the end for consistency, because
// here allocator is also always at end
constexpr U64 default_temp_allocator_arena_size = 4 * 1024;

struct Allocator {
    U8 *(*alloc_func)(void * allocator_data, U64 size, U64 alignment);
    void (*free_func)(void *allocator_data, void *memory);
    void *data;
};

inline U8 default_temp_allocator_arena_buffer[default_temp_allocator_arena_size] = {};
inline Arena default_temp_allocator_arena = {
    default_temp_allocator_arena_buffer,
    default_temp_allocator_arena_size, 0,
    BackingBufferType::ArrayU8
};

inline Allocator default_allocator = {&cstd_malloc_wrapper, &cstd_free_wrapper, NULL};
inline Allocator default_temp_allocator = {&arena_alloc_wrapper, &arena_free_wrapper, &default_temp_allocator_arena};

template<typename T = U8>
T *alloc(U64 count = JK_SINGLE_ELEMENT, Allocator *allocator = &default_allocator) {
    return allocator->alloc_func(allocator->data, count * sizeof(T), JK_AlignOf(T));
}

template<typename T>
void free(T *memory, Allocator *allocator = &default_allocator) {
    return allocator->free_func(allocator->data, memory);
}

} /* namespace base::mem */

#endif
