#ifndef JK_CONTAINERS_H
#define JK_CONTAINERS_H

// @TAG(jdk): slice
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

// @TAG(jdk): stack
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

// @TAG(jdk): dynamic array / list
#define JK_LIST_DEFAULT_START_CAPACITY 4

template <typename T>
struct List {
    T *buf;
    U64 len;
    U64 cap;
};

/* @TODO(jdk): add multiple ways to allocate */
template <typename T>
List<T> make_list(U64 start_capacity = JK_LIST_DEFAULT_START_CAPACITY) {
    return {(T *)malloc(start_capacity * sizeof(T)), 0, start_capacity};
}

template <typename T>
static void _list_ensure_init(List<T> *list) {
    if(list->buf == NULL) {
	*list = make_list<T>();
    }
}

template <typename T>
static void _list_ensure_cap_for_push(List<T> *list) {
    if(list->len + 1 > list->cap) {
	U64 new_cap = ceil(1.5 * (list->cap + 1));
	T *temp = list->buf;
	list->buf = (T *)malloc(new_cap * sizeof(T));
	memcpy(list->buf, temp, list->len * sizeof(T));
	list->cap = new_cap;
	free(temp);
    }
}

template <typename T>
T *list_push(List<T> *list, T val = {}) {
    assert(list);
    _list_ensure_init<T>(list);
    _list_ensure_cap_for_push<T>(list);
    T *new_elem = &list->buf[list->len];
    *new_elem = val;
    list->len++;
    return new_elem;
}

template <typename T>
T *list_push(List<T> *list, T *pval) {
    assert(list);
    _list_ensure_init<T>(list);
    _list_ensure_cap_for_push<T>(list);
    T *new_elem = &list->buf[list->len];
    if(pval == NULL) {
	*new_elem = {};
    } else {
	memcpy(new_elem, pval, sizeof(T));
    }
    list->len++;
    return new_elem;
}

template <typename T, typename ArgT>
T *list_find(List<T> *list, B8 (*fn_condition)(T *, ArgT), ArgT arg) {
    for(U64 i = 0; i < list->len; ++i) {
	T *current = &list->buf[i];
	if(fn_condition(current, arg)) {
	    return current;
	}
    }
    return NULL;
}

// @TAG(jdk): extrusive circular doubly linked list
template <typename T>
struct Link {
    Link<T> *next;
    Link<T> *prev;
    T val;
};

template <typename T>
T *link_push(Arena *arena, Link<T> **first, T val) {
    if(*first == NULL) {
	*first = arena_alloc<Link<T>>(arena);
	(*first)->next = *first;
	(*first)->prev = *first;
	(*first)->val = val;
	return &(*first)->val;
    } else {
	Link<T> *new_elem = arena_alloc<Link<T>>(arena);
	new_elem->prev = (*first)->prev;
	new_elem->next = *first;
	new_elem->val = val;
	// jdk: make last
	(*first)->prev->next = new_elem;
	(*first)->prev = new_elem;
	return &new_elem->val;
    }
}

template <typename T, typename ArgT>
T *link_find(Link<T> *first, B8 (*fn_condition)(T *, ArgT), ArgT arg) {
    if(first == NULL) {
	return NULL;
    }
    Link<T> *current = first;
    do {
	if(fn_condition(&current->val, arg)) {
	    return &current->val;
	}
	current = current->next;
    } while(current != first);
    return NULL;
}

#define JK_LinkForeach(first, name_current, content) do {\
    if(first == NULL) {\
	break;\
    }\
    decltype(first) current_link = (first);\
    do {\
	decltype(&(first)->val) name_current = &current_link->val;\
	content/*;*/\
	current_link = current_link->next;\
    } while(current_link != (first));\
} while(0)


#endif
