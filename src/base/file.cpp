#include "file.hpp"

// TODO(jdk): add Windows
B8 file_exists(Context *context, Str8 path) {
    struct stat st;
    return stat(cstr_copy_from_str8(context->frame_arena, path), &st) == -1;
}

FileDescriptor file_open(Context *context, Str8 path, FileOpenConfig config) {
    char *path_cstr = cstr_copy_from_str8(context->frame_arena, path);
    FileDescriptor result = open(path_cstr, (I32)config);
    if(result == -1) {
	printf("could not open file %s", path_cstr);
	exit(JK_ERROR_FILEOPEN);
    }
    return result;
}

U64 file_get_size(FileDescriptor fd) {
    struct stat st;
    if(fstat(fd, &st) == -1) {
	exit(JK_ERROR_FILESTATS);
    }
    return st.st_size;
}

U64 file_get_size_at_path(Context *context, Str8 path) {
    struct stat st;
    if(stat(cstr_copy_from_str8(context->frame_arena, path), &st) == -1) {
	exit(JK_ERROR_FILESTATS);
    }
    return st.st_size;
}

// jdk: buffer has to be allocated already
U64 file_read(FileDescriptor fd, void *buffer, U64 count) {
    if(count > JK_I64_MAX) {
	printf("64bit linux read has undefined behaviour for reads greater than I64_MAX");
	exit(JK_ERROR_FILEREAD);
    }
    I64 bytes_read = read(fd, buffer, count);
    if(bytes_read == -1) {
	exit(JK_ERROR_FILEREAD);
    }
    return (U64)bytes_read;
}

Str8 file_read_full_to_str8(Context *context, Str8 path) {
    FileDescriptor fd = file_open(context, path, FileOpenConfig_ReadOnly);
    U64 file_size = file_get_size(fd);
    Str8 str = str8_alloc_buffer(context->file_arena, file_size);
    if(file_read(fd, str.start, str.len) != file_size) {
	exit(JK_ERROR_FILEREAD);
    }
    file_close(fd);
    return str;
}

void file_close(FileDescriptor fd) {
    I32 result = close(fd);
    if(result == -1) {
	exit(JK_ERROR_FILECLOSE);
    }
}
