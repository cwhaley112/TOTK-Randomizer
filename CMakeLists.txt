cmake_minimum_required(VERSION 3.12)
project(randomizer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "-O2")
set(CMAKE_CXX_FLAGS_DEBUG "-g -Wall")

add_subdirectory(lib/oead)
add_subdirectory(lib/yaml-cpp)

file(GLOB_RECURSE csourcefiles "src/*.c")
file(GLOB_RECURSE cppsourcefiles "src/*.cpp")

add_executable(randomizer ${csourcefiles} ${cppsourcefiles})

target_include_directories(randomizer PUBLIC ${CMAKE_SOURCE_DIR}/include)

target_link_libraries(randomizer PUBLIC oead)
target_link_libraries(randomizer PRIVATE yaml-cpp)