cmake_minimum_required(VERSION 3.10)

project(utf8proc C)

add_library(utf8proc STATIC
    utf8proc.c
)

target_include_directories(utf8proc PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Required for some platforms (including Android)
target_compile_definitions(utf8proc PUBLIC UTF8PROC_STATIC)
