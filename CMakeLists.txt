cmake_minimum_required(VERSION 3.20)
project(TestLang)

set(CMAKE_CXX_STANDARD 17)

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" OR "x${CMAKE_CXX_SIMULATE_ID}" STREQUAL "xMSVC")
    add_compile_options(-W4)
else ()
    add_compile_options(
            -Wall
            -Wextra)
endif ()
add_compile_options(
        -Werror
        -Wno-unused-parameter)

add_compile_options(-D_USE_MATH_DEFINES -D_CRT_SECURE_NO_WARNINGS)

include_directories(src)

add_executable(
        TestLang
        src/Array.hpp
        src/Defines.hpp
        src/Lexer.cpp
        src/Lexer.hpp
        src/Main.cpp
        src/String.hpp
        src/Token.hpp src/Parser.cpp src/Parser.hpp src/Ast.hpp)
