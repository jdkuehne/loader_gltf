rm -r obj
mkdir obj
mkdir obj/base

CFLAGS="-ggdb -O0 -Wall -std=gnu++17 -DJK_COMPILER_GCC=1 -I ./src/"

g++ -c $CFLAGS -o obj/main.o      src/main.cpp

g++ -c $CFLAGS -o obj/base/core.o      src/base/core.cpp
g++ -c $CFLAGS -o obj/base/str.o       src/base/str.cpp
g++ -c $CFLAGS -o obj/base/file.o      src/base/file.cpp
g++ -c $CFLAGS -o obj/base/vec.o       src/base/vec.cpp
g++ -c $CFLAGS -o obj/base/quat.o      src/base/quat.cpp
g++ -c $CFLAGS -o obj/base/mat.o       src/base/mat.cpp
g++ -c $CFLAGS -o obj/base/arena.o     src/base/arena.cpp

g++ -c $CFLAGS -o obj/window.o    src/window.cpp
g++ -c $CFLAGS -o obj/shader.o    src/shader.cpp
g++ -c $CFLAGS -o obj/gltf_load.o src/gltf_load.cpp
g++ -c $CFLAGS -o obj/ui.o        src/ui.cpp

OBJS="obj/main.o obj/window.o obj/shader.o obj/gltf_load.o obj/ui.o\
    obj/base/core.o obj/base/str.o obj/base/file.o obj/base/vec.o obj/base/quat.o\
    obj/base/mat.o obj/base/arena.o"

# g++ -Wl,-rpath=./lib -o game $OBJS -L./lib -lglfw -lwr_tiny_gltf

g++ -o loader $OBJS -lglfw -lm

./loader
echo "game exited with code $?"
