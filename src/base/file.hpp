#ifndef FILE_H
#define FILE_H

#include "core.hpp"
#include "str.hpp"
#include "context.hpp"

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

typedef enum {
    FileOpenConfig_ReadOnly = O_RDONLY,

    FileOpenConfig_WriteOnly = O_WRONLY,
    FileOpenConfig_WriteOnly_Create = O_WRONLY | O_CREAT,
    FileOpenConfig_WriteOnly_Append = O_WRONLY | O_APPEND,
    FileOpenConfig_WriteOnly_Append_Create = O_WRONLY | O_APPEND | O_CREAT,

    FileOpenConfig_ReadWrite = O_RDWR,
    FileOpenConfig_ReadWrite_Create = O_RDWR | O_CREAT,
    FileOpenConfig_ReadWrite_Append = O_RDWR | O_APPEND,
    FileOpenConfig_ReadWrite_Append_Create = O_RDWR | O_APPEND | O_CREAT,
} FileOpenConfig;

typedef I32 FileDescriptor;

Context *get_file_context_ptr();

B8 file_exists(Context *context, Str8 path);
FileDescriptor file_open(Context *context, Str8 path, FileOpenConfig config);
U64 file_get_size(FileDescriptor fd);
U64 file_get_size_at_path(Context *context, Str8 path);
U64 file_read(FileDescriptor fd, void *buffer, U64 count);
Str8 file_read_full_to_str8(Context *context, Str8 path);
void file_close(FileDescriptor fd);

#endif
