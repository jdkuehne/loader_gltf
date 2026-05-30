#ifndef STR_H
#define STR_H

#include "core.hpp"
#include "arena.hpp"

#include <math.h>

typedef struct Str8 {
    U8 *start;
    U64 len;
} Str8;

B8 char_is_space(U8 c);
B8 char_is_lower(U8 c);
B8 char_is_upper(U8 c);
B8 char_is_alpha(U8 c);
B8 char_is_slash(U8 c);
B8 char_is_fslash(U8 c);
B8 char_is_bslash(U8 c);

Str8 str8(U8 *start, U64 len);
Str8 str8_cstr(const char *cstr);
// jdk: just a shorthand for str8_cstr
Str8 str8c(const char *cstr);
Str8 str8_range(U8 *first, U8 *one_past_last);

Str8 str8_alloc_buffer(Arena *arena, U64 len);
char *cstr_alloc_buffer(Arena *arena, U64 len);

U8 *str8_first(Str8 str);
U8 *str8_last(Str8 str);
U8 *str8_one_past_last(Str8 str);
U8 str8_first_char(Str8 str);
U8 str8_last_char(Str8 str);
U8 str8_one_past_last_char(Str8 str);
U8 *str8_first_not_of_space(Str8 str);
U8 *str8_last_not_of_space(Str8 str);
U8 *str8_one_past_last_not_of_space(Str8 str);

U8 str8_equal(Str8 lhs, Str8 rhs);

Str8 str8_trim(Str8 str);
Str8 str8_substr(Str8 str, U64 offset, U64 len);

Str8 str8_copy(Arena *arena, Str8 str);
Str8 str8_copy_cstr(Arena *arena, char *cstr);
char *cstr_copy_from_str8(Arena *arena, Str8 str);

Str8 str8_cat(Arena *arena, Str8 str1, Str8 str2);
Str8 str8_cat_cstrs(Arena *arena, char *cstr1, char *cstr2);

Str8 str8_dir_finish_with_slash(Arena *arena, Str8 dir);

Str8 str8_from_i64(Arena *arena, I64 x);
// jdk: adds thousands separators
Str8 str8_from_i64_pretty(Arena *arena, I64 x);
Str8 str8_from_f32(Arena *arena, F32 x, U64 decimal_places);

void put_str8(Str8 str);
void putln_str8(Str8 str);

#endif
