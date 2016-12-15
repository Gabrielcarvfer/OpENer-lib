project(OpENerMain2 CXX)

file(GLOB_RECURSE SRC_FILES src/*.cpp src/*.h)
add_executable(OpENerMain2 ${SRC_FILES})