#ifndef STR_H
#define STR_H

#include "core.hpp"
#include "mem/allocator.hpp"

struct Str8 {
    U8 *start;
    U64 len;
};

B8 char_is_space(U8 c);
B8 char_is_lower(U8 c);
B8 char_is_upper(U8 c);
B8 char_is_alpha(U8 c);
B8 char_is_slash(U8 c);
B8 char_is_fslash(U8 c);
B8 char_is_bslash(U8 c);

inline U64 cstr_length(const char *cstr) { U64 i = 0; while(cstr[i] != 0) i++; return i; }

inline Str8 str8(U8 *start, U64 len) { return {start, len}; }
inline Str8 str8_cstr(const char *cstr) { return {(U8 *)cstr, cstr_length(cstr)}; }
inline Str8 str8c(const char *cstr) { return str8_cstr(cstr); }
inline Str8 str8_range(U8 *first, U8 *one_past_last) {
    return {first, (U64)(one_past_last - first)};
}

Str8 str8_alloc_buffer(U64 len, Allocator *allocator = &default_allocator);
char *cstr_alloc_buffer(U64 len, Allocator *allocator = &default_allocator);

U8 *str8_first(Str8 str);
U8 *str8_last(Str8 str);
U8 *str8_one_past_last(Str8 str);
U8 str8_first_char(Str8 str);
U8 str8_last_char(Str8 str);
U8 str8_one_past_last_char(Str8 str);
U8 *str8_first_not_of_space(Str8 str);
U8 *str8_last_not_of_space(Str8 str);
U8 *str8_one_past_last_not_of_space(Str8 str);
U8 *str8_first_of_space(Str8 str);

U8 str8_equal(Str8 lhs, Str8 rhs);

Str8 str8_trim(Str8 str);
Str8 str8_substr(Str8 str, U64 offset, U64 len);

Str8 str8_copy(Str8 str, Allocator *allocator = &default_allocator);
char *cstr_copy_from_str8(Str8 str, Allocator *allocator = &default_allocator);

Str8 str8_cat(Str8 str1, Str8 str2, Allocator *allocator = &default_allocator);
Str8 str8_cat_cstrs(char *cstr1, char *cstr2, Allocator *allocato = &default_allocator);

Str8 str8_dir_finish_with_slash(Str8 dir, Allocator *allocator = &default_allocator);

Str8 str8_from_i64(I64 x, Allocator *allocator = &default_allocator);
// jdk: adds thousands separators
Str8 str8_from_i64_pretty(I64 x, Allocator *allocator = &default_allocator);
#if 0
Str8 str8_from_f32(Arena *arena, F32 x, U64 decimal_places);
#endif

void put_str8(Str8 str);
void putln_str8(Str8 str);

#endif
