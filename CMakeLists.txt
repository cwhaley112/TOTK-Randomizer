cmake_minimum_required(VERSION 3.12)
project(randomizer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(lib/oead)

file(GLOB_RECURSE csourcefiles "src/*.c")
file(GLOB_RECURSE cppsourcefiles "src/*.cpp")

add_executable(randomizer ${csourcefiles} ${cppsourcefiles})

target_include_directories(randomizer PUBLIC ${CMAKE_SOURCE_DIR}/include)
# target_include_directories(randomizer PUBLIC /opt/boost_1_82_0/boost)

target_link_libraries(randomizer PUBLIC oead)