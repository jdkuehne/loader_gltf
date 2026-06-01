#ifndef FILE_H
#define FILE_H

#include "../core.hpp"
#include "../str.hpp"
#include "../allocator.hpp"

#if JK_OS_LINUX
# include "file_types_linux.hpp"
#elif JK_OS_WIN32
# include "file_types_win32.hpp"
#else
# error "no defined file api for this OS"
#endif

namespace base
{

B8 file_exists(Str8 path, Allocator *temp_allocator = &default_temp_allocator);
FileDescriptor file_open(Str8 path, FileAccessFlag access_flag, FileCreateFlag create_flag,
	Allocator *temp_allocator = &default_temp_allocator);
U64 file_get_size(FileDescriptor fd);
U64 file_get_size(Str8 path, Allocator *temp_allocator = &default_temp_allocator);
U64 file_read(FileDescriptor fd, void *buffer, U64 count);
Str8 file_read_full_to_str8(Str8 path, Allocator *file_allocator = &default_allocator,
	Allocator *temp_allocator = &default_temp_allocator);
void file_close(FileDescriptor fd);

}

#endif
