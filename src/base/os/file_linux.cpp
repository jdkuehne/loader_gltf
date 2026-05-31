#include "file.hpp"

B8 file_exists(Str8 path, Allocator temp_allocator) {
    struct stat st;
    return stat(cstr_copy_from_str8(path, temp_allocator), &st) == -1;
}

FileDescriptor file_open(Str8 path, FileAccessFlag access_flag, FileCreateFlag create_flag,
	Allocator temp_allocator) {
    char *path_cstr = cstr_copy_from_str8(path, temp_allocator);
    FileDescriptor result = open(path_cstr, (int)access_flag || (int)create_flag);
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

U64 file_get_size_at_path(Str8 path, Allocator temp_allocator) {
    struct stat st;
    if(stat(cstr_copy_from_str8(path, temp_allocator), &st) == -1) {
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

Str8 file_read_full_to_str8(Str8 path, Allocator file_allocator, Allocator temp_allocator) {
    FileDescriptor fd = file_open(path, FileAccessFlag::ReadOnly, FileCreateFlag::NoCreate,
	    temp_allocator);
    U64 file_size = file_get_size(fd);
    Str8 str = str8_alloc_buffer(file_size, file_allocator);
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
