#ifndef JK_LIST_H
#define JK_LIST_H

#define JK_DEFAULT_START_CAPACITY 4

template <typename T>
struct List {
    T *buf;
    U64 len;
    U64 cap;
};

/* TODO(jdk): add multiple ways to allocate
enum class ListAllocType {
    Heap,
};
*/
template <typename T>
List<T> make_list(U64 start_capacity = JK_DEFAULT_START_CAPACITY) {
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

#endif
