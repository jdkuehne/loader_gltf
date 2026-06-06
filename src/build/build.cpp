#include "base/str.hpp"
#include "base/mem/allocator.hpp"
#include "base/containers.hpp"

#define JK_MAX_SOURCE_FILES 64

Str8 cmd_append(Str8 cmd, Str8 to_append) {
    if(!char_is_space(*str8_last(cmd))) {
	cmd = str8_cat(cmd, str8c(" "), &default_temp_allocator);
    }
    cmd = str8_cat(cmd, to_append, &default_temp_allocator);
    return cmd;
}

U64 str8_separate_by_space(Stack<Str8> *result, Str8 x) {
    U8 *elem_first = str8_first_not_of_space(x);
    if(elem_first == NULL)
	return result->len;
    Str8 trimmed_at_start = str8_range(elem_first, str8_one_past_last(x));
    U8 *elem_one_past_last = str8_first_of_space(trimmed_at_start);
    if(elem_one_past_last == NULL) {
	Str8 elem = str8_range(elem_first, str8_one_past_last(trimmed_at_start));
	stack_push(result, elem);
	return result->len;
    } else {
	Str8 elem = str8_range(elem_first, elem_one_past_last);
	stack_push(result, elem);
	str8_separate_by_space(result, str8_range(str8_first(trimmed_at_start),
		    str8_one_past_last(trimmed_at_start)));
    }
}

int main() {
    Str8 cmd = str8c("cl");
    Stack<Str8> source_files = make_stack<Str8>(JK_MAX_SOURCE_FILES);
    str8_separate_by_space(&source_files, str8c(
		"main.cpp base/os/file_win32.cpp "
		"base/vec.cpp base/quat.cpp base/mat.cpp "
		"base/mem/arena.cpp base/mem/c_alloc.cpp "
		"window.cpp shader.cpp gltf_load.cpp ui.cpp"));

    return 0;
}
