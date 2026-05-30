#include "core.hpp"

//##################################################
// jdk: alignment, pow2
B8 is_pow2(U64 x) {
    return x != 0 && (x & (x - 1)) == 0;
}

B8 is_pow2_or_zero(U64 x) {
    return (x & (x - 1)) == 0;
}

U64 align_pow2(U64 pos, U64 alignment_pow2) {
    // TODO(jdk): assert based on some #define?
    // NOTE(jdk): this only works for powers of two!
    U64 modulo_mask = ~(alignment_pow2 - 1);
    return (pos + alignment_pow2 - 1) & modulo_mask;
}

//##################################################
// jdk: number ranges
F32 min(F32 a, F32 b) {
    return (a < b) ? a : b;
}

F32 max(F32 a, F32 b) {
    return (a > b) ? a : b;
}

F32 clamp(F32 x, F32 min, F32 max) {
    return (x < min) ? min : ((x > max) ? max : x);
}
