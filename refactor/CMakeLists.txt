cmake_minimum_required(VERSION 3.1)

project(nnn VERSION 1.0
    DESCRIPTION "Naive Neural Network"
    LANGUAGES C)

find_package(OpenMP)
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")

add_library(nnn_math_vector STATIC src/math/vector.c)
target_include_directories(nnn_math_vector PUBLIC include)
# https://github.com/SixTrack/SixTrack/blob/master/CMakeLists.txt
set(CMAKE_C_FLAGS "-mavx -mavx2 ${CMAKE_C_FLAGS}")

add_executable(test_basic test/basic.c)
target_link_libraries(test_basic nnn_math_vector)

add_executable(test_vector_addition test/vector_addition.c)
target_link_libraries(test_vector_addition nnn_math_vector)

add_test(NAME test_basic
    COMMAND test_basic)

