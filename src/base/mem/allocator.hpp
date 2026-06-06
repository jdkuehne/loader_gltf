#ifndef JK_ALLOCATOR_H
#define JK_ALLOCATOR_H

#include "arena.hpp"
#include "c_alloc.hpp"

#include <string.h>

// @TODO(jdk): maybe change so allocators have their data at the end for consistency, because
// here allocator is also always at end
constexpr U64 default_temp_allocator_arena_size = 256 * 1024;

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
T *mem_alloc(Allocator *allocator = &default_allocator) {
    T *new_memory = (T *)allocator->alloc_func(allocator->data, sizeof(T), JK_AlignOf(T));
    memset(new_memory, 0, sizeof(T));
    return new_memory;
}

template<typename T = U8>
T *mem_alloc(U64 count, Allocator *allocator = &default_allocator) {
    T *new_memory = (T *)allocator->alloc_func(allocator->data, count * sizeof(T), JK_AlignOf(T));
    memset(new_memory, 0, count * sizeof(T));
    return new_memory;
}

template<typename T>
void mem_free(T *memory, Allocator *allocator = &default_allocator) {
    return allocator->free_func(allocator->data, memory);
}

#endif
