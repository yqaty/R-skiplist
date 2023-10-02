cmake_minimum_required(VERSION 3.22)

project(Rskiplist)

set(CMAKE_CXX_STANDARD 20)

aux_source_directory(./src DIR_SRC)

add_executable(Rskiplist ${DIR_SRC} ./test/test.cpp)

target_include_directories(Rskiplist PRIVATE ./include)