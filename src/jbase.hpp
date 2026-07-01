#ifndef JBASE_HPP
#define JBASE_HPP

#if defined(_WIN32)
# define JM_OS_WIN32
#elif defined(__linux__)
# define JM_OS_LINUX
#else
# error "invalid os"
#endif

#if defined(__GNUC__)
# define JM_COMPILER_GCC
#elif defined(__clang__)
# define JM_COMPILER_CLANG
#elif defined(_MSC_VER)
# define JM_COMPILER_MSVC
#else
# error "invalid compiler"
#endif

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <string.h>
#include <stdarg.h>

#if defined(JM_OS_WIN32)
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

// NOTE(jdk): used for allocators and stuff for one, I simply prefer the look of it
#define JM_SINGLE_ELEMENT 1

#define jm_arrlen(arr) sizeof(arr)/sizeof((arr)[0])

#define JM_INT8_MAX  (~((int8_t)1  << 7))
#define JM_INT16_MAX (~((int16_t)1 << 15))
#define JM_INT32_MAX (~((int32_t)1 << 31))
#define JM_INT64_MAX (~((int64_t)1 << 63))

#define JM_UINT8_MAX  (~((uint8_t)0))
#define JM_UINT16_MAX (~((uint16_t)0))
#define JM_UINT32_MAX (~((uint32_t)0))
#define JM_UINT64_MAX (~((uint64_t)0))

#define JM_PI32   3.14159f
// TODO(jdk): more precise for f64?
#define JM_PI64   3.14159

#define jm_sq(x) ((x)*(x))

#define jm_rad32(deg) ((deg) * JM_PI32/180.0f)
#define jm_rad64(deg) ((deg) * JM_PI64/180.0)

#define jm_kib(x) (1024 * (x))
#define jm_mib(x) (1024 * 1024 * (x))

//##################################################
// jdk: alignment, pow2 checks
#if defined(JM_COMPILER_MSVC)
# define jm_alignof(T) __alignof(T)
#elif defined(JM_COMPILER_CLANG)
# define jm_alignof(T) __alignof(T)
#elif defined(JM_COMPILER_GCC)
# define jm_alignof(T) __alignof__(T)
#endif

#if defined(JM_COMPILER_GCC)
# define jm_typeof(T) typeof(T)
#elif defined(JM_COMPILER_MSVC)
# define jm_typeof(T) __typeof(T)__
#endif

#if defined(JM_COMPILER_GCC)
# define jm_forceinline __attribute__((always_inline))
#elif defined(JM_COMPILER_MSVC)
# define jm_forceinline __forceinline
#else
# error "no known force inline intrinsic for this compiler"
#endif

#if defined(JM_COMPILER_GCC)
# define jm_trap() __builtin_trap()
#elif defined(JM_COMPILER_MSVC)
# define jm_trap() __debugbreak()
#endif

inline float min(float a, float b) { return (a < b) ? a : b; }
inline float max(float a, float b) { return (a > b) ? a : b; }
inline float clamp(float x, float min, float max) {
    return (x < min) ? min : ((x > max) ? max : x);
}

// jdk: uint64 so can be used with allocators etc.
inline bool is_pow2(uint64_t x) { return x != 0 && (x & (x - 1)) == 0; }
inline bool is_pow2_or_zero(uint64_t x) { return (x & (x - 1)) == 0; }
inline uint64_t align_pow2(uint64_t pos, uint64_t alignment) {
    assert(is_pow2(alignment) && "cannot align if not power of two");
    uint64_t modulo_mask = ~(alignment - 1);
    return (pos + alignment - 1) & modulo_mask;
}

inline uint64_t compose_uint(uint32_t low, uint32_t high) {
    return ((uint64_t)low | ((uint64_t)high << 32));
}


// @TAG(jdk): Error codes
#define JM_SUCCESS 0
#define JM_ERROR_GENERAL 1
//file errors
#define JM_ERROR_FILEOPEN  2
#define JM_ERROR_FILESTATS 3
#define JM_ERROR_FILEREAD  4
#define JM_ERROR_FILECLOSE 5
//Shader compilation errors
#define JM_ERROR_SHADERCOMPILE_VS 10
#define JM_ERROR_SHADERCOMPILE_GS 11
#define JM_ERROR_SHADERCOMPILE_FS 12
#define JM_ERROR_SHADERPROGRAMLINK 15
//gl/glfw setup
#define JM_ERROR_LOADGLPROCS 20
#define JM_ERROR_WINDOWCREATE 21
//model loading
#define JM_ERROR_MODEL_LOAD 30
#define JM_ERROR_UNKNOWN_PRIMITIVE_ATTRIB 35
//allocation errors
#define JM_ERROR_UNINITIALIZED_ARENA 50
#define JM_ERROR_INVALID_ARENA_PTR 51

// @TAG(jdk): allocators

enum class BackingBufferType {
    HeapAlloc,
    ArrayUint8,
};

struct Arena {
    void *memory;
    size_t size;
    size_t offset;
    BackingBufferType type;
};

// jdk: default is created via malloc
// // @TODO(jdk): currently zero at creation and when allocating, is that fine?
Arena make_arena(size_t size, uint8_t clear_value = 0);
Arena make_arena_from_uint8_array(uint8_t *backing_buffer, size_t size, uint8_t clear_value = 0);
uint8_t *arena_alloc(Arena *arena, size_t size, size_t alignment);
void arena_free(Arena *arena, void *allocation);
void arena_reset(Arena *arena);
void destroy_arena(Arena *arena);

// jdk: mem_alloc allocator wrappers
inline uint8_t *arena_alloc_wrapper(void *arena, size_t size, size_t alignment) {
    return arena_alloc((Arena *)arena, size, alignment);
}

inline void arena_free_wrapper(void *arena, void *allocation) {
    return arena_free((Arena *)arena, allocation);
}

inline uint8_t *cstd_malloc_wrapper(void *data, size_t size, size_t alignment) {
    return (uint8_t *)malloc(size);
}

inline void cstd_free_wrapper(void *data, void *memory) {
    return free(memory);
}

// @TAG(jdk): universal allocator
#define JM_TEMP_ALLOCATOR_ARENA_SIZE jm_kib(256)

typedef uint8_t *(*AllocFn)(void *, size_t, size_t);
typedef void (*FreeFn)(void *, void *);

struct Allocator {
    AllocFn alloc_fn;
    FreeFn free_fn;
    void *data = NULL;
    // jdk: if this is false zero_alloc will zero itself
    bool does_zero_internally = false;
};

inline uint8_t temp_allocator_buffer[JM_TEMP_ALLOCATOR_ARENA_SIZE] = {};
inline Arena temp_allocator_arena = make_arena_from_uint8_array(temp_allocator_buffer,
	jm_arrlen(temp_allocator_buffer), '#');

inline Allocator cstd_allocator = {&cstd_malloc_wrapper, &cstd_free_wrapper};
inline Allocator temp_allocator = {&arena_alloc_wrapper, &arena_free_wrapper, &temp_allocator_arena};

inline void _zero_if_not_internally(void *memory, size_t size, Allocator *allocator) {
    if(!allocator->does_zero_internally) {
	memset(memory, 0, size);
    }
}

template <typename T = uint8_t, bool zero = false>
T *alloc(size_t count, Allocator *allocator = &cstd_allocator) {
    size_t alloc_size = count * sizeof(T);
    T *memory = (T *)allocator->alloc_fn(allocator->data, alloc_size, jm_alignof(T));
    if constexpr(zero) {
	_zero_if_not_internally(memory, alloc_size, allocator);
    }
    return memory;
}

template <typename T = uint8_t, bool zero = false>
inline T *alloc(Allocator *allocator = &cstd_allocator) {
    return alloc<T, zero>(JM_SINGLE_ELEMENT, allocator);
}

template <typename T = uint8_t>
inline T *zero_alloc(size_t count, Allocator *allocator = &cstd_allocator) {
    return alloc<T, true>(count, allocator);
}

template <typename T = uint8_t>
inline T *zero_alloc(Allocator *allocator = &cstd_allocator) {
    return alloc<T, true>(JM_SINGLE_ELEMENT, allocator);
}

void release(void *memory, Allocator *allocator = &cstd_allocator);

// @TAG(jdk): String/Char

struct Str8 {
    uint8_t *ptr;
    size_t len;
};

bool char_is_space(uint8_t c);
bool char_is_lower(uint8_t c);
bool char_is_upper(uint8_t c);
bool char_is_alpha(uint8_t c);
bool char_is_slash(uint8_t c);
bool char_is_fslash(uint8_t c);
bool char_is_bslash(uint8_t c);

size_t cstr_length(const char *cstr);
char *cstr_alloc_buffer(size_t len, Allocator *allocator = &cstd_allocator);
char *cstr_from_str8(Str8 str, Allocator *allocator = &cstd_allocator);

Str8 str8(uint8_t *ptr, size_t len);
Str8 str8(const char *cstr);
Str8 str8(uint8_t *begin, uint8_t *end);
Str8 str8_alloc_buffer(size_t len, Allocator *allocator = &cstd_allocator);

uint8_t *str8_first(Str8 str);
uint8_t *str8_last(Str8 str);
uint8_t *str8_end(Str8 str);
uint8_t *str8_first_of_space(Str8 str);
uint8_t *str8_last_of_space(Str8 str);
uint8_t *str8_first_not_of_space(Str8 str);
uint8_t *str8_last_not_of_space(Str8 str);
uint8_t str8_equal(Str8 lhs, Str8 rhs);

Str8 str8_trim(Str8 str);
Str8 str8_substr(Str8 str, size_t offset, size_t len);
Str8 str8_copy(Str8 str, Allocator *allocator = &cstd_allocator);
Str8 str8_cat(Str8 str_l, Str8 str_r, Allocator *allocator = &cstd_allocator);
Str8 str8_cat(Str8 str_l, const char *str_r, Allocator *allocator = &cstd_allocator);
Str8 str8_cat(const char *str_l, Str8 str_r, Allocator *allocator = &cstd_allocator);
Str8 str8_cat(const char *str_l, const char *str_r, Allocator *allocator = &cstd_allocator);
Str8 str8_dir_finish_with_slash(Str8 dir, Allocator *allocator = &cstd_allocator);

// jdk: _a means explicit allocator => va args make default values impossible
struct FmtParams {
    Str8 str;
    Allocator *allocator = NULL;
};
struct CFmtParams {
    const char *cstr;
    Allocator *allocator = NULL;
};
typedef CFmtParams VFmtParams;

Str8 str8_vfmt(VFmtParams fmt, ...);
Str8 str8_fmt(FmtParams fmt, ...);
Str8 str8_cfmt(CFmtParams fmt, ...);

void str8_put(Str8 str);
void str8_putln(Str8 str);
#define jm_println(fmt, ...) str8_putln(str8_fmt((fmt) __VA_OPT__(,) __VA_ARGS__))
#define jm_cprintln(fmt, ...) str8_putln(str8_cfmt((fmt) __VA_OPT__(,) __VA_ARGS__))

// @TAG(jdk): file handling

#if defined(JM_OS_LINUX)

using FileDescriptor = int;

enum class FileAccessFlag {
    ReadWrite = O_RDWR,
    WriteOnly = O_WRONLY,
    ReadOnly = O_RDONLY,
};

enum class FileCreateFlag {
    CreateIfNotExist = O_CREAT,
    NoCreateTruncate = O_TRUNC,
    CreateOrTruncate = O_CREAT || O_TRUNC,
    NoCreate = 0,
};

#elif defined(JM_OS_WIN32)

using FileDescriptor = HANDLE;

enum class FileAccessFlag : DWORD {
    ReadWrite = GENERIC_READ || GENERIC_WRITE,
    WriteOnly = GENERIC_WRITE,
    ReadOnly = GENERIC_READ,
};

enum class FileCreateFlag : DWORD {
    CreateIfNotExist = CREATE_NEW,
    NoCreateTruncate = TRUNCATE_EXISTING,
    CreateOrTruncate = CREATE_NEW || TRUNCATE_EXISTING,
    NoCreate = OPEN_EXISTING,
};

#endif

bool file_exists(Str8 path);
FileDescriptor file_open(Str8 path, FileAccessFlag access_flag, FileCreateFlag create_flag);
size_t file_get_size(FileDescriptor fd);
size_t file_get_size(Str8 path);
size_t file_read(FileDescriptor fd, void *buffer, size_t count);
Str8 file_read_full_to_str8(Str8 path, Allocator *file_allocator = &cstd_allocator);
void file_close(FileDescriptor fd);

#endif

#ifdef JBASE_IMPLEMENTATION

// @TAG(jdk): arena impl
Arena make_arena(size_t size, uint8_t clear_value) {
    Arena result = {0};
    result.memory = malloc(size);
    result.size = size;
    result.type = BackingBufferType::HeapAlloc;
    memset(result.memory, clear_value, size);
    return result;
}

Arena make_arena_from_uint8_array(uint8_t *backing_buffer, size_t size, uint8_t clear_value) {
    Arena result = {0};
    result.memory = backing_buffer;
    result.size = size;
    result.type = BackingBufferType::ArrayUint8;
    memset(result.memory, clear_value, size);
    return result;
}

uint8_t *arena_alloc(Arena *arena, size_t size, size_t alignment) {
    if(arena == NULL)
	exit(JM_ERROR_INVALID_ARENA_PTR);
    assert(is_pow2(alignment));
    uintptr_t aligned_ptr = align_pow2((size_t)arena->memory + arena->offset, alignment);
    size_t aligned_offset = aligned_ptr - (size_t)arena->memory;
    assert(aligned_offset + size <= arena->size);
    uint8_t *result = (uint8_t *)aligned_ptr;
    arena->offset = aligned_offset + size;
    return result;
}

void arena_free(Arena *arena, void *allocation) {
    // jdk: don't do shit
}

void arena_reset(Arena *arena) {
    arena->offset = 0;
}

void destroy_arena(Arena *arena) {
    if(arena->type == BackingBufferType::HeapAlloc) {
	free(arena->memory);
    }
    arena->memory = NULL;
    arena->size = 0;
    arena->offset = 0;
    // jdk: type doesn't matter
}

// @TAG(jdk): alloc impl
void release(void *memory, Allocator *allocator) {
    allocator->free_fn(allocator->data, memory);
}

// @TAG(jdk): string impl

bool char_is_space(uint8_t c) {
    // jdk: space       htab         newline      vtab         form feed    carriage return
    return c == 0x20 || c == 0x09 || c == 0x0A || c == 0x0B || c == 0x0C || c == 0x0D;
}

bool char_is_lower(uint8_t c) {
    return 'a' <= c && c <= 'z';
}

bool char_is_upper(uint8_t c) {
    return 'A' <= c && c <= 'Z';
}

bool char_is_alpha(uint8_t c) {
    return char_is_lower(c) || char_is_upper(c);
}

bool char_is_slash(uint8_t c) {
    return c == '\\' || c == '/';
}

bool char_is_fslash(uint8_t c) {
    return c == '/';
}

bool char_is_bslash(uint8_t c) {
    return c == '\\';
}

size_t cstr_length(const char *cstr) {
    size_t i = 0;
    for(; cstr[i] != 0; ++i)
	assert(i < JM_UINT64_MAX);
    return i;
}

char *cstr_alloc_buffer(size_t len, Allocator *allocator) {
    return alloc<char>(len + 1, allocator);
}

char *cstr_from_str8(Str8 str, Allocator *allocator) {
    char *cstr = cstr_alloc_buffer(str.len, allocator);
    memcpy(cstr, str.ptr, str.len);
    cstr[str.len] = 0;
    return cstr;
}

char *cstr_cat(const char *str_l, const char *str_r, Allocator *allocator = &cstd_allocator) {
    size_t len_l = strlen(str_l);
    size_t len_r = strlen(str_r);
    size_t newlen = len_l + len_r;
    char *result = alloc<char>(newlen, allocator);
    memcpy(result, str_l, len_l);
    memcpy(result + len_l, str_r, len_r);
    result[newlen] = '\0';
    return result;
}

Str8 str8(uint8_t *ptr, size_t len) {
    return {ptr, len};
}

Str8 str8(const char *cstr) {
    return {(uint8_t *)cstr, cstr_length(cstr)};
}

Str8 str8(uint8_t *begin, uint8_t *end) {
    return {begin, (size_t)(end - begin)};
}

Str8 str8_alloc_buffer(size_t len, Allocator *allocator) {
    return str8(alloc(len, allocator), len);
}

uint8_t *str8_first(Str8 str) {
    return str.ptr;
}
uint8_t *str8_last(Str8 str) {
    return str.ptr + str.len - 1;
}
uint8_t *str8_end(Str8 str) {
    return str.ptr + str.len;
}

uint8_t *str8_first_of_space(Str8 str) {
    for(uint8_t *it = str8_first(str); it != str8_end(str); ++it) {
	if(char_is_space(*it)) {
	    return it;
	}
    }
    return NULL;
}

uint8_t *str8_first_not_of_space(Str8 str) {
    for(uint8_t *it = str8_first(str); it <= str8_last(str); ++it) {
	if(!char_is_space(*it)) {
	    return it;
	}
    }

    return NULL;
}

uint8_t *str8_last_of_space(Str8 str) {
    for(; str.len > 0; --str.len) {
	uint8_t *c = str8_last(str);
	if(char_is_space(*c)) {
	    return c;
	}
    }
    return NULL;
}

uint8_t *str8_last_not_of_space(Str8 str) {
    for(; str.len > 0; --str.len) {
	uint8_t *c = str8_last(str);
	if(!char_is_space(*c)) {
	    return c;
	}
    }
    return NULL;
}

uint8_t str8_equal(Str8 lhs, Str8 rhs) {
    if(lhs.len != rhs.len) {
	return false;
    } else {
	for(size_t i = 0; i < lhs.len; ++i) {
	    if(lhs.ptr[i] != rhs.ptr[i]) {
		return false;
	    }
	}
	return true;
    }
}

Str8 str8_trim(Str8 str) {
    Str8 result = {0};
    uint8_t *first = str8_first_not_of_space(str);
    uint8_t *end = str8_last_not_of_space(str) + 1;
    if(!first || !end) {
	return result;
    }
    return str8(first, end);
}

Str8 str8_substr(Str8 str, size_t offset, size_t len) {
    return str8(str.ptr + offset, len);
}

Str8 str8_copy(Str8 str, Allocator *allocator) {
    Str8 result = str8_alloc_buffer(str.len, allocator);
    memcpy(result.ptr, str.ptr, str.len);
    return result;
}

Str8 str8_cat(Str8 str_l, Str8 str_r, Allocator *allocator) {
    Str8 result = str8_alloc_buffer(str_l.len + str_r.len, allocator);
    memcpy(result.ptr, str_l.ptr, str_l.len);
    memcpy(result.ptr + str_l.len, str_r.ptr, str_r.len);
    return result;
}
Str8 str8_cat(Str8 str_l, const char *str_r, Allocator *allocator) {
    size_t len_r = strlen(str_r);
    Str8 result = str8_alloc_buffer(str_l.len + len_r, allocator);
    memcpy(result.ptr, str_l.ptr, str_l.len);
    memcpy(result.ptr + str_l.len, str_r, len_r);
    return result;
}
Str8 str8_cat(const char *str_l, Str8 str_r, Allocator *allocator) {
    size_t len_l = strlen(str_l);
    Str8 result = str8_alloc_buffer(len_l + str_r.len, allocator);
    memcpy(result.ptr, str_l, len_l);
    memcpy(result.ptr + len_l, str_r.ptr, str_r.len);
    return result;
}
Str8 str8_cat(const char *str_l, const char *str_r, Allocator *allocator) {
    size_t len_l = strlen(str_l);
    size_t len_r = strlen(str_r);
    Str8 result = str8_alloc_buffer(len_l + len_r, allocator);
    memcpy(result.ptr, str_l, len_l);
    memcpy(result.ptr + len_l, str_r, len_r);
    return result;
}

Str8 str8_dir_finish_with_slash(Str8 dir, Allocator *allocator) {
    Str8 trimmed = str8_trim(dir);
    uint8_t last = *str8_last(trimmed);
    if(char_is_slash(last)) {
	return trimmed;
    } else {
	return str8_cat(trimmed, str8("/"), allocator);
    }
}

Str8 str8_vfmt(VFmtParams fmt, va_list args) {
    int len = vsnprintf(NULL, 0, fmt.cstr, args);
    int bufsize = len + 1;
    char *buf = alloc<char>(bufsize, fmt.allocator ? fmt.allocator : &cstd_allocator);
    vsnprintf(buf, bufsize, fmt.cstr, args);
    return str8(buf);
}

Str8 str8_cfmt(CFmtParams fmt, ...) {
    va_list args;
    va_start(args, fmt);
    Str8 result = str8_vfmt(fmt, args);
    va_end(args);
    return result;
}

Str8 str8_fmt(FmtParams fmt, ...) {
    va_list args;
    va_start(args, fmt);
    char *fmt_cstr = cstr_from_str8(fmt.str, &cstd_allocator);
    Str8 result = str8_vfmt({fmt_cstr, fmt.allocator}, args);
    release(fmt_cstr, &cstd_allocator);
    va_end(args);
    return result;
}

void str8_put(Str8 str) {
    for(size_t i = 0; i < str.len; ++i) {
	putchar(str.ptr[i]);
    }
}

void str8_putln(Str8 str) {
    for(size_t i = 0; i < str.len; ++i) {
	putchar(str.ptr[i]);
    }
    putchar('\n');
}

// @TAG(jdk): file handling

#if defined(JM_OS_WIN32)

bool file_exists(Str8 path) {
    return (GetFileAttributesA(cstr_from_str8(path, &temp_allocator)) != INVALID_FILE_ATTRIBUTES);
}

FileDescriptor file_open(Str8 path, FileAccessFlag access_flag, FileCreateFlag create_flag) {
    char *path_cstr = cstr_from_str8(path, &temp_allocator);
    HANDLE result = CreateFileA(path_cstr, (DWORD)access_flag, 0 /*share*/, NULL,
	    (DWORD)create_flag, FILE_ATTRIBUTE_NORMAL, NULL);
    if(result == INVALID_HANDLE_VALUE) {
	printf("could not open file %s", path_cstr);
	exit(JM_ERROR_FILEOPEN);
    }
    return result;
}

size_t file_get_size(FileDescriptor fd) {
    BY_HANDLE_FILE_INFORMATION file_information = {};
    if(!GetFileInformationByHandle(fd, &file_information)) {
	exit(JM_ERROR_FILESTATS);
    }
    return compose_uint(file_information.nFileSizeLow, file_information.nFileSizeHigh);
}

size_t file_get_size_at_path(Str8 path) {
    BY_HANDLE_FILE_INFORMATION file_information = {};
    FileDescriptor fd = file_open(path, FileAccessFlag::ReadOnly,
	    FileCreateFlag::NoCreate);
    if(!GetFileInformationByHandle(fd, &file_information)) {
	exit(JM_ERROR_FILESTATS);
    }
    file_close(fd);
    return compose_uint(file_information.nFileSizeLow, file_information.nFileSizeHigh);
}

// jdk: buffer has to be allocated already
// @TODO(jdk): async alternative
size_t file_read(FileDescriptor fd, void *buffer, size_t count) {
    if(count > JM_INT64_MAX) {
	// @TODO(jdk): edit for windows
	printf("64bit linux read has undefined behaviour for reads greater than I64_MAX");
	exit(JM_ERROR_FILEREAD);
    }
    DWORD bytes_read = 0;
    if(ReadFile(fd, buffer, count, &bytes_read, NULL) == FALSE) {
	exit(JM_ERROR_FILEREAD);
    }
    // @TODO(jdk): make this explicit that readfile reads max 2^32 bytes
    return (size_t)bytes_read;
}

// jdk: this one's identical to the linux version
Str8 file_read_full_to_str8(Str8 path, Allocator *file_allocator) {
    FileDescriptor fd = file_open(path, FileAccessFlag::ReadOnly, FileCreateFlag::NoCreate);
    size_t file_size = file_get_size(fd);
    Str8 str = str8_alloc_buffer(file_size, file_allocator);
    if(file_read(fd, str.ptr, str.len) != file_size) {
	exit(JM_ERROR_FILEREAD);
    }
    file_close(fd);
    return str;
}

void file_close(FileDescriptor fd) {
    if(!CloseHandle(fd)) {
	exit(JM_ERROR_FILECLOSE);
    }
}

#endif // jdk: file win32

#endif
