#ifndef JK_STACK_H
#define JK_STACK_H

template <typename T>
struct Stack {
    T *buf;
    U64 len;
    U64 cap;
};

template <typename T>
Stack<T> make_stack(Arena *arena, U64 cap) {
    return {arena_alloc<T>(arena, cap), 0, cap};
}

template <typename T>
T *stack_push(Stack<T> *stack, T elem) {
    assert(stack->len + 1 <= stack->cap);
    T *new_pos = stack->buf + stack->len;
    *new_pos = elem;
    stack->len++;
    return new_pos;
}

template <typename T, typename ArgT>
T *stack_find(Stack<T> *stack, B8 (*fn_condition)(T *, ArgT), ArgT arg) {
    for(U64 i = 0; i < stack->len; ++i) {
	T *current = &stack->buf[i];
	if(fn_condition(current, arg)) {
	    return current;
	}
    }
    return NULL;
}

#endif
