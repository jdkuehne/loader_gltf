#ifndef JK_FILE_TYPES_LINUX_H
#define JK_FILE_TYPES_LINUX_H

# include <fcntl.h>
# include <sys/stat.h>
# include <unistd.h>

// @TODO(jdk): make sure that int is ok
using FileDescriptor = int;

enum class FileAccessFlag {
    ReadWrite = O_RDWR,
    WriteOnly = O_WRONLY,
    ReadOnly = O_RDONLY,
};

enum class FileCreateFlag {
    CreateIfNotExist = O_CREAT,
    NoCreateTruncate = O_TRUNC,
    CreateOrTruncate = O_CREAT || O_TRUNC,
    NoCreate = 0,
};

#endif
