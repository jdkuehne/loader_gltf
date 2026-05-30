#include "str.hpp"

//##################################################
// jdk: char characterisation
B8 char_is_space(U8 c) {
    // jdk: space       htab         newline      vtab         form feed    carriage return
    return c == 0x20 || c == 0x09 || c == 0x0A || c == 0x0B || c == 0x0C || c == 0x0D;
}

B8 char_is_lower(U8 c) {
    return 'a' <= c && c <= 'z';
}

B8 char_is_upper(U8 c) {
    return 'A' <= c && c <= 'Z';
}

B8 char_is_alpha(U8 c) {
    return char_is_lower(c) || char_is_upper(c);
}

B8 char_is_slash(U8 c) {
    return c == '\\' || c == '/';
}

B8 char_is_fslash(U8 c) {
    return c == '/';
}

B8 char_is_bslash(U8 c) {
    return c == '\\';
}


//##################################################
// jdk: string functions
Str8 str8(U8 *start, U64 len) {
    Str8 result = {start, len};
    return result;
}

Str8 str8_cstr(const char *cstr) {
    return str8((U8 *)cstr, strlen(cstr));
}

Str8 str8c(const char *cstr) {
    return str8_cstr(cstr);
}

Str8 str8_range(U8 *first, U8 *one_past_last) {
    return str8(first, (U64)(one_past_last - first));
}

Str8 str8_alloc_buffer(Arena *arena, U64 len) {
    return str8(arena_alloc<U8>(arena, len), len);
}

char *cstr_alloc_buffer(Arena *arena, U64 len) {
    return arena_alloc<char>(arena, len + 1);
}

U8 *str8_first(Str8 str) {
    return str.start;
}
U8 *str8_last(Str8 str) {
    return str.start + str.len - 1;
}
U8 *str8_one_past_last(Str8 str) {
    return str.start + str.len;
}

U8 str8_first_char(Str8 str) {
    return *str.start;
}
U8 str8_last_char(Str8 str) {
    return str.start[str.len - 1];
}
U8 str8_one_past_last_char(Str8 str) {
    return str.start[str.len];
}
U8 *str8_first_not_of_space(Str8 str) {
    for(U8 *it = str8_first(str); it <= str8_last(str); ++it) {
	if(!char_is_space(*it)) {
	    return it;
	}
    }

    return NULL;
}
U8 *str8_last_not_of_space(Str8 str) {
    for(; str.len > 0; --str.len) {
	U8 c = str8_last_char(str);
	if(!char_is_space(c)) {
	    return str8_last(str);
	}
    }
    return NULL;
}
U8 *str8_one_past_last_not_of_space(Str8 str) {
    for(; str.len > 0; --str.len) {
	U8 c = str8_last_char(str);
	if(!char_is_space(c)) {
	    return str8_one_past_last(str);
	}
    }
    return NULL;
}

U8 str8_equal(Str8 lhs, Str8 rhs) {
    if(lhs.len != rhs.len) {
	return JK_FALSE;
    } else {
	for(U64 i = 0; i < lhs.len; ++i) {
	    if(lhs.start[i] != rhs.start[i]) {
		return JK_FALSE;
	    }
	}
	return JK_TRUE;
    }
}


Str8 str8_trim(Str8 str) {
    Str8 result = {0};
    U8 *first = str8_first_not_of_space(str);
    U8 *one_past_last = str8_one_past_last_not_of_space(str);
    if(!first || !one_past_last) {
	return result;
    }
    return str8_range(first, one_past_last);
}

Str8 str8_substr(Str8 str, U64 offset, U64 len) {
    return str8(str.start + offset, len);
}

Str8 str8_copy(Arena *arena, Str8 str) {
    Str8 result = str8_alloc_buffer(arena, str.len);
    memcpy(result.start, str.start, str.len);
    return result;
}

Str8 str8_copy_cstr(Arena *arena, char *cstr) {
    return str8_copy(arena, str8_cstr(cstr));
}

char *cstr_copy_from_str8(Arena *arena, Str8 str) {
    char *cstr = cstr_alloc_buffer(arena, str.len);
    memcpy(cstr, str.start, str.len);
    cstr[str.len] = 0;
    return cstr;
}

Str8 str8_cat(Arena *arena, Str8 str1, Str8 str2) {
    Str8 result = str8_alloc_buffer(arena, str1.len + str2.len);
    memcpy(result.start, str1.start, str1.len);
    memcpy(result.start + str1.len, str2.start, str2.len);
    return result;
}

Str8 str8_cat_cstrs(Arena *arena, char *cstr1, char *cstr2) {
    return str8_cat(arena, str8_cstr(cstr1), str8_cstr(cstr2));
}

Str8 str8_dir_finish_with_slash(Arena *arena, Str8 dir) {
    Str8 trimmed = str8_trim(dir);
    U8 last = str8_last_char(trimmed);
    if(char_is_slash(last)) {
	return trimmed;
    } else {
	return str8_cat(arena, trimmed, str8_cstr("/"));
    }
}

static U64 uint_get_num_digits(U64 x) {
    U64 result = 1;
    U64 next_digit_min = 10;
    while(!(x < next_digit_min)) {
	result++;
	next_digit_min *= 10;
    }
    return result;
}

Str8 str8_from_i64(Arena *arena, I64 x) {
    // jdk: figured this shit out myself let's goo
    B8 is_negative = x < 0;
    U64 x_abs = is_negative ? (U64)(-x) : (U64)x;
    U64 num_digits = uint_get_num_digits(x_abs);
    // printf("%d\n", num_digits);

    Str8 str = str8_alloc_buffer(arena, num_digits + (is_negative ? 1 : 0));
    
    U64 processed = 0;
    U64 digit = 0;
    U64 digit_min = 1;
    do {
	U64 val_this_digit = (x_abs - processed) % (digit_min * 10);
	str.start[(str.len - 1) - digit] = '0' + (val_this_digit / digit_min);

	digit_min *= 10;
	digit++;
	processed += val_this_digit;
    } while(processed < x_abs);

    if(is_negative) {
	str.start[0] = '-';
    }
    return str;
}

// jdk: adds thousands separators
Str8 str8_from_i64_pretty(Arena *arena, I64 x) {
    // jdk: figured this shit out myself let's goo
    B8 is_negative = x < 0;
    U64 x_abs = is_negative ? (U64)(-x) : (U64)x;
    U64 num_digits = uint_get_num_digits(x_abs);

    U64 num_thousands = (num_digits - 1) / 3;
    printf("num_thousands: %lu\n", num_thousands);

    Str8 str = str8_alloc_buffer(arena, num_digits + (is_negative ? 1 : 0) + num_thousands);
    
    U64 processed = 0;
    U64 digit = 0;
    U64 digit_min = 1;
    U64 num_separators = 0;;
    do {
	if(digit != 0 && digit % 3 == 0) {
	    str.start[(str.len - 1) - digit - num_separators] = '\'';
	    num_separators++;
	}

	U64 val_this_digit = (x_abs - processed) % (digit_min * 10);
	str.start[(str.len - 1) - digit - num_separators] = '0' + (val_this_digit / digit_min);


	digit_min *= 10;
	digit++;
	processed += val_this_digit;
    } while(processed < x_abs);

    if(is_negative) {
	str.start[0] = '-';
    }
    return str;
}

#if 0
Str8 str8_from_f32(Arena *arena, F32 x, U64 decimal_places) {
    I64 before_point = (I64)x;
    // jdk: still negative

    //TODO(jdk): sometimes some of the later places are slightly incorrect, 
    //figure out if it's possible to remove this problem
    I64 after_point = (I64)((x - floorf(x)) * powf(10.f, decimal_places));

    // jdk: also applies to after point
    B8 is_negative = before_point < 0;
    U64 part_l = (is_negative) ? -before_point : before_point;
    U64 part_r = (is_negative) ? -after_point : after_point;

    U64 num_digits_l = uint_get_num_digits(part_l);
    U64 num_digits_r = decimal_places;
    // printf("%d\n", num_digits);

    U64 bufsize = num_digits_l + 1 + num_digits_r + (is_negative ? 1 : 0);
    Str8 str = str8_alloc_buffer(arena, bufsize);

    U64 offset_l = is_negative ? 1 : 0;
    U64 len_l = num_digits_l;
    Str8 str_l = str8_substr(str, offset_l, len_l);

    U64 offset_dot = offset_l + len_l;

    U64 offset_r = offset_dot + 1;
    U64 len_r = num_digits_r;
    Str8 str_r = str8_substr(str, offset_r, len_r);

    Str8 strs[2] = {str_l, str_r};
    I64 parts[2] = {part_l, part_r};

    for(int i = 0; i < 2; ++i) {
	U64 processed = 0;
	U64 digit = 0;
	U64 digit_min = 1;
	do {
	    U64 val_this_digit = (parts[i] - processed) % (digit_min * 10);
	    strs[i].start[(strs[i].len - 1) - digit] = '0' + (val_this_digit / digit_min);

	    digit_min *= 10;
	    digit++;
	    processed += val_this_digit;
	} while(processed < parts[i]);
    }
    if(part_r == 0) {
	memset(str_r.start, '0', str_r.len);
    }

    if(decimal_places > 0) {
	str.start[offset_dot] = '.';
    }

    if(is_negative) {
	str.start[0] = '-';
    }
    return str;
}
#endif
// Str8 str8_from_uint(U64 x);


void put_str8(Str8 str) {
    for(U64 i = 0; i < str.len; ++i) {
	putchar(str.start[i]);
    }
}

void putln_str8(Str8 str) {
    for(U64 i = 0; i < str.len; ++i) {
	putchar(str.start[i]);
    }
    putchar('\n');
}
