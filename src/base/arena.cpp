#include "arena.hpp"

Arena make_arena(U64 size) {
    Arena result = {0};
    result.memory = malloc(size);
    result.size = size;
    result.type = BackingBuffer_HeapAlloc;
    return result;
}

// TODO(jdk): check if this double copy is optimized, else make helper init function where
// the arena is passed by reference (pointer), or macro
Arena make_arena_cleared(U64 size, U8 clear_value) {
    Arena result = make_arena(size);
    memset(result.memory, clear_value, size);
    return result;
}

Arena make_arena_from_u8_array(U8 *backing_buffer, U64 size) {
    Arena result = {0};
    result.memory = backing_buffer;
    result.size = size;
    result.type = BackingBuffer_ArrayU8;
    return result;
}

Arena make_arena_from_u8_array_cleared(U8 *backing_buffer, U64 size, U8 clear_value) {
    Arena result = make_arena_from_u8_array(backing_buffer, size);
    memset(result.memory, clear_value, size);
    return result;
}

void *arena_alloc(Arena *arena, U64 size, U64 alignment_pow2) {
    if(!arena) { exit(JK_ERROR_INVALID_ARENA_PTR); }
    assert(is_pow2(alignment_pow2));
    U64 aligned_offset = align_pow2((U64)arena->memory + arena->offset, alignment_pow2) - (U64)arena->memory;
    assert(aligned_offset + size <= arena->size);
    void *result = (U8 *)arena->memory + aligned_offset;
    arena->offset = aligned_offset + size;
    // jdk: zero memory
    memset(result, 0, size);
    return result;
}

void arena_free(Arena *arena, void *allocation) {
    // jdk: don't do shit
}

void arena_reset(Arena *arena) {
    arena->offset = 0;
}

void destroy_arena(Arena *arena) {
    if(arena->type == BackingBuffer_HeapAlloc) {
	free(arena->memory);
    }
    arena->memory = NULL;
    arena->size = 0;
    arena->offset = 0;
    // jdk: type doesn't matter
}

ArenaSavepoint arena_make_savepoint(Arena *arena) {
    if(!arena) { exit(JK_ERROR_INVALID_ARENA_PTR); }
    ArenaSavepoint result = { arena, arena->offset };
    return result;
}

void arena_savepoint_reset(ArenaSavepoint *savepoint) {
    savepoint->arena->offset = savepoint->offset;
    memset(savepoint, 0, sizeof(*savepoint));
}
