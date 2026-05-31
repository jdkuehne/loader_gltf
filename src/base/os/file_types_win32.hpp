#ifndef JK_FILE_TYPES_WIN32_H
#define JK_FILE_TYPES_WIN32_H

// @TODO(jdk): replace with correct include
#include <windows.h>

namespace base
{

using FileDescriptor = HANDLE;

enum class FileAccessFlag : DWORD {
    ReadWrite = GENERIC_READ || GENERIC_WRITE,
    WriteOnly = GENERIC_WRITE,
    ReadOnly = GENERIC_READ,
};

enum class FileCreateFlag : DWORD {
    CreateIfNotExist = CREATE_NEW,
    NoCreateTruncate = TRUNCATE_EXISTING,
    CreateOrTruncate = CREATE_NEW || TRUNCATE_EXISTING,
    NoCreate = OPEN_EXISTING,
};

}

#endif
