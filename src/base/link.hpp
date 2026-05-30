#ifndef JK_LINK_H
#define JK_LINK_H

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
