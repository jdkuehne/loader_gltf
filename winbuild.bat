@echo off

if not exist ".\bin\" (
    mkdir bin
    copy /y .\lib\glfw3.dll .\bin\
)

set MODE=O

if %MODE%==DEBUG (
    set CFLAGS=/nologo /EHsc /wd4201 /wd4505 /W4 /std:c++20 /Zi
) else (
    set CFLAGS=/nologo /EHsc /wd4201 /wd4505 /W4 /std:c++20 /O2
)

cd bin

cl /c %CFLAGS% ../src/main.cpp
cl /c %CFLAGS% ../src/window.cpp
cl /c %CFLAGS% ../src/shader.cpp
cl /c %CFLAGS% ../src/gltf_load.cpp
cl /c %CFLAGS% ../src/ui.cpp

cl /nologo /MDd /Fe:loader main.obj window.obj shader.obj gltf_load.obj ui.obj ../lib/glfw3dll.lib /link /DEBUG

cd ..
