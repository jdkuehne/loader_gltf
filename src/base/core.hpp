#ifndef CORE_H
#define CORE_H

#include <stdint.h>

//##################################################
// jdk: attributes
#if JK_COMPILER_GCC
# define jk_forceinline __attribute__((always_inline))
#else
# error "no known force inline intrinsic for this compiler"
#endif

//##################################################
// jdk: base types
typedef int8_t   I8;
typedef int16_t  I16;
typedef int32_t  I32;
typedef int64_t  I64;

typedef uint8_t  U8;
typedef uint16_t U16;
typedef uint32_t U32;
typedef uint64_t U64;

typedef float    F32;
typedef double   F64;

typedef int8_t    B8;
typedef int16_t   B16;
typedef int32_t   B32;
typedef int64_t   B64;

// NOTE(jdk): used for allocators and stuff for one, I simply prefer the look of it
#define JK_SINGLE_ELEMENT 1

//##################################################
// jdk: Slices and Arrays (templates and instantiations)
// TODO(jdk): maybe change this slice stuff
#define JK_ArrayLength(arr) sizeof(arr)/sizeof((arr)[0])

#define JK_GenType_Slice(T)\
    typedef struct Slice##T {\
	T *ptr;\
	U64 len;\
    } Slice##T
#define JK_GenFnSig_slice(T)\
    Slice##T JK_CONSTRUCTOR_Slice##T(T *ptr, U64 len)
#define JK_GenFn_slice(T)\
    JK_GenFnSig_slice(T) {\
	Slice##T result = {ptr, len};\
	return result;\
    }

#define JK_SliceFromArray(arr) JK_CONSTRUCTOR_Slice##typeof((arr)[0])((arr), JK_ArrayLength(arr))

#define JK_StackPush(stack, x) do {\
    assert((stack)->len + 1 <= (stack)->cap);\
    (stack)->buf[(stack)->len] = (x);\
    (stack)->len++;\
} while(0)

//##################################################
// jdk: constants
#define JK_TRUE  1
#define JK_FALSE 0

#define JK_I8_MAX  (~((I8)1  << 7))
#define JK_I16_MAX (~((I16)1 << 15))
#define JK_I32_MAX (~((I32)1 << 31))
#define JK_I64_MAX (~((I64)1 << 63))

#define JK_U8_MAX  (~((U8)0))
#define JK_U16_MAX (~((U16)0))
#define JK_U32_MAX (~((U32)0))
#define JK_U64_MAX (~((U64)0))

//##################################################
// jdk: random math stuff
#define JK_PI32   3.14159f
// TODO(jdk): more precise for f64?
#define JK_PI64   3.14159

#define JK_Rad32(deg) ((deg) * JK_PI32/180.0f)
#define JK_Rad64(deg) ((deg) * JK_PI64/180.0)

#define JK_KiB(x) 1024 * (x)
#define JK_MiB(x) 1024 * 1024 * (x)

//##################################################
// jdk: number ranges
F32 min(F32 a, F32 b);
F32 max(F32 a, F32 b);
F32 clamp(F32 x, F32 min, F32 max);

// NOTE(jdk): only macros are prefixed with JK because those wouldn't get pointed out
// as second definition or anything by compiler

//##################################################
// jdk: alignment, pow2 checks
#if JK_COMPILER_MSVC
# define JK_AlignOf(T) __alignof(T)
#elif JK_COMPILER_CLANG
# define JK_AlignOf(T) __alignof(T)
#elif JK_COMPILER_GCC
# define JK_AlignOf(T) __alignof__(T)
#else
# error "no known alignof macro for this compiler."
#endif

#if JK_COMPILER_GCC
# define JK_TypeOf(T) typeof(T)
#else
# error "no known typeof intrinsic for this compiler"
#endif

// jdk: in U64 because equal to size-type
B8 is_pow2(U64 x);
B8 is_pow2_or_zero(U64 x);
U64 align_pow2(U64 pos, U64 alignment_pow2);

#if JK_COMPILER_GCC
# define JK_Trap() __builtin_trap()
#else
# error "no trap for this compiler"
#endif

//##################################################
// jdk: Error codes
#define JK_SUCCESS 0
#define JK_ERROR_GENERAL 1
//file errors
#define JK_ERROR_FILEOPEN  2
#define JK_ERROR_FILESTATS 3
#define JK_ERROR_FILEREAD  4
#define JK_ERROR_FILECLOSE 5
//...
//Shader compilation errors
#define JK_ERROR_SHADERCOMPILE_VS 10
#define JK_ERROR_SHADERCOMPILE_GS 11
#define JK_ERROR_SHADERCOMPILE_FS 12
//...
#define JK_ERROR_SHADERPROGRAMLINK 15
//gl/glfw setup
#define JK_ERROR_LOADGLPROCS 20
#define JK_ERROR_WINDOWCREATE 21
//model loading
#define JK_ERROR_MODEL_LOAD 30
#define JK_ERROR_UNKNOWN_PRIMITIVE_ATTRIB 35
// jdk: allocation errors
#define JK_ERROR_UNINITIALIZED_ARENA 50
#define JK_ERROR_INVALID_ARENA_PTR 51

#endif // CORE_H
