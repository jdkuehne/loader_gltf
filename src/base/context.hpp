#ifndef CONTEXT_H
#define CONTEXT_H

#include "arena.hpp"

typedef struct Context {
    Arena *frame_arena;
    Arena *persistent_arena;
    Arena *file_arena;
    void *(*heap_alloc)(U64);
    void (*heap_free)(void *);
} Context;

#endif
