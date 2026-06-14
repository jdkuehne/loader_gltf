@echo off

cd bin

cl /c /nologo /EHsc /Zi /std:c++20 /DJK_COMPILER_MSVC=1 /DJK_OS_WIN32=1 ../src/main.cpp
cl /c /nologo /EHsc /Zi /std:c++20 /DJK_COMPILER_MSVC=1 /DJK_OS_WIN32=1 ../src/window.cpp
cl /c /nologo /EHsc /Zi /std:c++20 /DJK_COMPILER_MSVC=1 /DJK_OS_WIN32=1 ../src/shader.cpp
cl /c /nologo /EHsc /Zi /std:c++20 /DJK_COMPILER_MSVC=1 /DJK_OS_WIN32=1 ../src/gltf_load.cpp
cl /c /nologo /EHsc /Zi /std:c++20 /DJK_COMPILER_MSVC=1 /DJK_OS_WIN32=1 ../src/ui.cpp

cl /c /nologo /EHsc /Zi /std:c++20 /DJK_COMPILER_MSVC=1 /DJK_OS_WIN32=1 ../src/base/str.cpp
cl /c /nologo /EHsc /Zi /std:c++20 /DJK_COMPILER_MSVC=1 /DJK_OS_WIN32=1 ../src/base/os/file_win32.cpp
cl /c /nologo /EHsc /Zi /std:c++20 /DJK_COMPILER_MSVC=1 /DJK_OS_WIN32=1 ../src/base/vec.cpp
cl /c /nologo /EHsc /Zi /std:c++20 /DJK_COMPILER_MSVC=1 /DJK_OS_WIN32=1 ../src/base/quat.cpp
cl /c /nologo /EHsc /Zi /std:c++20 /DJK_COMPILER_MSVC=1 /DJK_OS_WIN32=1 ../src/base/mat.cpp
cl /c /nologo /EHsc /Zi /std:c++20 /DJK_COMPILER_MSVC=1 /DJK_OS_WIN32=1 ../src/base/mem/arena.cpp
cl /c /nologo /EHsc /Zi /std:c++20 /DJK_COMPILER_MSVC=1 /DJK_OS_WIN32=1 ../src/base/mem/c_alloc.cpp

cl /nologo /MDd /Fe:loader main.obj window.obj shader.obj gltf_load.obj ui.obj str.obj ^
file_win32.obj vec.obj quat.obj mat.obj arena.obj c_alloc.obj ../lib/glfw3dll.lib /link /DEBUG

cd ..
