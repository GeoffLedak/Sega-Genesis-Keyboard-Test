cmake_minimum_required(VERSION 3.8)
project(keyboardTest)

set(CMAKE_C_STANDARD 99)

set(SOURCE_FILES main.c font.c crt0.s hw_md.h hw_md.s sys.h)
add_executable(keyboardTest ${SOURCE_FILES})