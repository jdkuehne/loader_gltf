#ifndef JK_SLICE_H
#define JK_SLICE_H

template <typename T>
struct Slice {
    T *ptr;
    U64 len;
};

template <typename T>
Slice<T> make_slice(T *ptr, U64 len) {
    return {ptr, len};
}

#define JK_SliceForeach(x, name_current, content) do {\
    for(decltype((x)->ptr) name_current = (x)->ptr; name_current != (x)->ptr + (x)->len; ++name_current) {\
	content\
    }\
} while(0)

#endif
