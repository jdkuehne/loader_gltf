#include "file.hpp"

#include <stdio.h>

B8 file_exists(Str8 path, Allocator *temp_allocator) {
    return (GetFileAttributesA(cstr_copy_from_str8(path, temp_allocator)) != INVALID_FILE_ATTRIBUTES);
}

FileDescriptor file_open(Str8 path, FileAccessFlag access_flag, FileCreateFlag create_flag,
	Allocator *temp_allocator) {
    char *path_cstr = cstr_copy_from_str8(path, temp_allocator);
    HANDLE result = CreateFileA(path_cstr, (DWORD)access_flag, 0 /*share*/, NULL,
	    (DWORD)create_flag, FILE_ATTRIBUTE_NORMAL, NULL);
    if(result == INVALID_HANDLE_VALUE) {
	printf("could not open file %s", path_cstr);
	exit(JK_ERROR_FILEOPEN);
    }
    return result;
}

U64 file_get_size(FileDescriptor fd) {
    BY_HANDLE_FILE_INFORMATION file_information = {};
    if(!GetFileInformationByHandle(fd, &file_information)) {
	exit(JK_ERROR_FILESTATS);
    }
    return compose_uint(file_information.nFileSizeLow, file_information.nFileSizeHigh);
}

U64 file_get_size_at_path(Str8 path, Allocator *temp_allocator) {
    BY_HANDLE_FILE_INFORMATION file_information = {};
    FileDescriptor fd = file_open(path, FileAccessFlag::ReadOnly,
	    FileCreateFlag::NoCreate, temp_allocator);
    if(!GetFileInformationByHandle(fd, &file_information)) {
	exit(JK_ERROR_FILESTATS);
    }
    file_close(fd);
    return compose_uint(file_information.nFileSizeLow, file_information.nFileSizeHigh);
}

// jdk: buffer has to be allocated already
// @TODO(jdk): async alternative
U64 file_read(FileDescriptor fd, void *buffer, U64 count) {
    if(count > JK_I64_MAX) {
	// @TODO(jdk): edit for windows
	printf("64bit linux read has undefined behaviour for reads greater than I64_MAX");
	exit(JK_ERROR_FILEREAD);
    }
    DWORD bytes_read = 0;
    if(ReadFile(fd, buffer, count, &bytes_read, NULL) == FALSE) {
	exit(JK_ERROR_FILEREAD);
    }
    // @TODO(jdk): make this explicit that readfile reads max 2^32 bytes
    return (U64)bytes_read;
}

// jdk: this one's identical to the linux version
Str8 file_read_full_to_str8(Str8 path, Allocator *file_allocator, Allocator *temp_allocator) {
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
    if(!CloseHandle(fd)) {
	exit(JK_ERROR_FILECLOSE);
    }
}
