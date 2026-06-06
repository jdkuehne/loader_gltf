#ifndef ARENA_H
#define ARENA_H

#include "../core.hpp"

enum class BackingBufferType {
    HeapAlloc,
    ArrayU8,
};

struct Arena {
    void *memory;
    U64 size;
    U64 offset;
    BackingBufferType type;
};

// TODO(jdk): add resize allocation? add prev offset to know if the allocation to
// resize is the latest one (in that case you could just increase offset and return the same pointer)

typedef struct ArenaSavepoint {
    Arena *arena;
    U64 offset;
} ArenaSavepoint;

// jdk: default is created via malloc
Arena make_arena(U64 size);
Arena make_arena_cleared(U64 size, U8 clear_value);
Arena make_arena_from_u8_array(U8 *backing_buffer, U64 size);
Arena make_arena_from_u8_array_cleared(U8 *backing_buffer, U64 size, U8 clear_value);

// TODO(jdk): allocations are zeroed always currently, might make a parameter
U8 *arena_alloc(Arena *arena, U64 size, U64 alignment);
inline U8 *arena_alloc_wrapper(void *arena, U64 size, U64 alignment) {
    return arena_alloc((Arena *)arena, size, alignment);
}

void arena_free(Arena *arena, void *allocation);
inline void arena_free_wrapper(void *arena, void *allocation) {
    return arena_free((Arena *)arena, allocation);
}
void arena_reset(Arena *arena);
void destroy_arena(Arena *arena);
ArenaSavepoint arena_make_savepoint(Arena *arena);
void arena_savepoint_reset(ArenaSavepoint *savepoint);

#endif
