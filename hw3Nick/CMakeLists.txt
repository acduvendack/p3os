cmake_minimum_required (VERSION 2.8)
project(hw3)

set(CMAKE_C_FLAGS "-std=c11 -Wall -Wextra -Wshadow -Werror -D_XOPEN_SOURCE=500")
set(CMAKE_CXX_FLAGS "-std=c++11 -Wall -Wextra -Wshadow -Werror -Wno-sign-compare -D_XOPEN_SOURCE=500")
set(CMAKE_BUILD_TYPE Debug)

include_directories("${PROJECT_SOURCE_DIR}/include")

# build a dynamic library called libblock_store.so
# note that the prefix lib will be automatically added in the filename.
add_library(libblock_store.so SHARED ${CMAKE_CURRENT_SOURCE_DIR}/src/block_store.c ${CMAKE_CURRENT_SOURCE_DIR}/include/block_store.h ${CMAKE_CURRENT_SOURCE_DIR}/src/bitmap.c ${CMAKE_CURRENT_SOURCE_DIR}/include/bitmap.h)


# make an executable
add_executable(${PROJECT_NAME}_test test/tests.cpp)
target_compile_definitions(${PROJECT_NAME}_test PRIVATE)
target_link_libraries(${PROJECT_NAME}_test gtest pthread libblock_store.so)
