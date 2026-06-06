#include "base/core.hpp"
#include "base/str.hpp"

#include <stdlib.h>
#include <stdio.h>

Str8 cmd_append(Str8 cmd, Str8 to_append) {
    if(!char_is_space(*str8_last(cmd))) {
	cmd = str8_cat(cmd, str8c(" "), &default_temp_allocator);
    }
    cmd = str8_cat(cmd, to_append, &default_temp_allocator);
    return cmd;
}

int main() {
    Str8 src_dir = str8c("src/");
    Str8 bin_dir = str8c("bin/");
    Str8 compiler = str8c("cl /c");
    Str8 linker = str8c("cl");
    Str8 cflags = str8c("/nologo /EHsc /Zi /std:c++20 /DJK_COMPILER_MSVC=1 /DJK_OS_WIN32=1");
    Str8 lflags = str8c("/nologo /MDd");
    const char *src_files[] = {
	"main.cpp", "window.cpp", "shader.cpp", "gltf_load.cpp", "ui.cpp",
	"base/str.cpp", "base/os/file_win32.cpp", "base/vec.cpp", "base/quat.cpp",
	"base/mat.cpp", "base/mem/arena.cpp", "base/c_alloc.cpp"
    };
    for(U64 i = 0; i < JK_ArrayLength(src_files); ++i) {
	Str8 src = str8_cat(src_dir, str8c(src_files[i]));
	Str8 comp_cmd = compiler;
	comp_cmd = cmd_append(comp_cmd, cflags);
	comp_cmd = cmd_append(comp_cmd, src);
	const char *comp_cmd_cstr = cstr_copy_from_str8(comp_cmd);
	puts(comp_cmd_cstr);
	//system(comp_cmd_str);
    }
}
