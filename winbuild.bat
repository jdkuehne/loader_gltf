@echo off

cd bin

cl /c /nologo /EHsc /Zi /std:c++20 ../src/main.cpp
cl /c /nologo /EHsc /Zi /std:c++20 ../src/window.cpp
cl /c /nologo /EHsc /Zi /std:c++20 ../src/shader.cpp
cl /c /nologo /EHsc /Zi /std:c++20 ../src/gltf_load.cpp
cl /c /nologo /EHsc /Zi /std:c++20 ../src/ui.cpp

cl /nologo /MDd /Fe:loader main.obj window.obj shader.obj gltf_load.obj ui.obj ../lib/glfw3dll.lib /link /DEBUG

cd ..
