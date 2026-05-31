rm -recurse obj
mkdir obj
mkdir obj/base

# $CFLAGS="\/EHsc /Zi /std:c++17 /D:JK_COMPILER_MSVC=1 /I./src/"

cd obj

cl /c /nologo /EHsc /Z7 /std:c++20 /DJK_COMPILER_MSVC=1 /DJK_OS_WIN32=1 ../src/main.cpp

cl /c /nologo /EHsc /Z7 /std:c++20 /DJK_COMPILER_MSVC=1 /DJK_OS_WIN32=1 ../src/base/core.cpp
cl /c /nologo /EHsc /Z7 /std:c++20 /DJK_COMPILER_MSVC=1 /DJK_OS_WIN32=1 ../src/base/str.cpp
cl /c /nologo /EHsc /Z7 /std:c++20 /DJK_COMPILER_MSVC=1 /DJK_OS_WIN32=1 ../src/base/os/file_win32.cpp
cl /c /nologo /EHsc /Z7 /std:c++20 /DJK_COMPILER_MSVC=1 /DJK_OS_WIN32=1 ../src/base/vec.cpp
cl /c /nologo /EHsc /Z7 /std:c++20 /DJK_COMPILER_MSVC=1 /DJK_OS_WIN32=1 ../src/base/quat.cpp
cl /c /nologo /EHsc /Z7 /std:c++20 /DJK_COMPILER_MSVC=1 /DJK_OS_WIN32=1 ../src/base/mat.cpp
cl /c /nologo /EHsc /Z7 /std:c++20 /DJK_COMPILER_MSVC=1 /DJK_OS_WIN32=1 ../src/base/arena.cpp

cl /c /nologo /EHsc /Z7 /std:c++20 /DJK_COMPILER_MSVC=1 /DJK_OS_WIN32=1 ../src/window.cpp
cl /c /nologo /EHsc /Z7 /std:c++20 /DJK_COMPILER_MSVC=1 /DJK_OS_WIN32=1 ../src/shader.cpp
cl /c /nologo /EHsc /Z7 /std:c++20 /DJK_COMPILER_MSVC=1 /DJK_OS_WIN32=1 ../src/gltf_load.cpp
cl /c /nologo /EHsc /Z7 /std:c++20 /DJK_COMPILER_MSVC=1 /DJK_OS_WIN32=1 ../src/ui.cpp

$OBJS="obj/main.o obj/window.o obj/shader.o obj/gltf_load.o obj/ui.o\
    obj/base/core.o obj/base/str.o obj/base/file.o obj/base/vec.o obj/base/quat.o\
    obj/base/mat.o obj/base/arena.o"

cd ..

cl /Fe:loader $OBJS
