cmake_minimum_required(VERSION 3.10)

set(EXECUTABLE_OUTPUT_PATH "bin")
set(CMAKE_CXX_FLAGS "-O3")
SET(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_FLAGS "-O3")
SET(CMAKE_C_STANDARD 11)
set(CMAKE_BUILD_TYPE RelWithDebInfo)

project(midi3mus)
include_directories(./src/)

add_executable(${CMAKE_PROJECT_NAME}
src/convert.c
src/main.c
src/midimerge.c
src/parsemidi.c
src/smalloc.c
)

target_link_libraries(midi3mus)