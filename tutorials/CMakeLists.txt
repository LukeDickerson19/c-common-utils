cmake_minimum_required(VERSION 3.20) # minimum cmake version (found w/ cmd "cmake --version", currently v4.1.1)
project(MyProject C) # The project name is MyProject, and the primary language is C

# set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD 11) # set c langange version, 11 is the version of c from the year 2011, it maps to the gcc flag "-std=c11"
# set(CMAKE_C_STANDARD 17)
# set(CMAKE_C_STANDARD 23)

# Add include directories w/ header files
include_directories(include)

# Add source files
add_executable(myprogram
    src/chatgpt.c
    src/helper.c)

# Optionally add subdirectories
# add_subdirectory(tests)
