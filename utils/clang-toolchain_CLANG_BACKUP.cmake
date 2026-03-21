# clang-toolchain.cmake
# Cross-platform Clang toolchain with sanitizers
# Supports Windows (Ninja) and Linux/macOS

# Specify compilers
set(CMAKE_C_COMPILER clang CACHE STRING "C compiler" FORCE)
set(CMAKE_CXX_COMPILER clang++ CACHE STRING "C++ compiler" FORCE)

# Set C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Default build type
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the build type" FORCE)
endif()

# Sanitizer options
option(ENABLE_SANITIZERS "Enable Address/UB/LeakSanitizers" OFF)

if(ENABLE_SANITIZERS)
    if(CMAKE_C_COMPILER_ID STREQUAL "Clang")
        message(STATUS "Sanitizers ENABLED")

        # AddressSanitizer, LeakSanitizer, UndefinedBehaviorSanitizer
        add_compile_options(-fsanitize=address -fsanitize=undefined -fno-omit-frame-pointer)
        add_link_options(-fsanitize=address -fsanitize=undefined)
        
        # Optional: detect leaks automatically
        set(ENV{ASAN_OPTIONS} "detect_leaks=1:abort_on_error=1:symbolize=1")
        set(ENV{UBSAN_OPTIONS} "print_stacktrace=1")
    else()
        message(WARNING "Sanitizers are only fully supported with Clang")
    endif()
endif()

# Strict warnings
add_compile_options(-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wformat=2)

# Enable runtime errors in Debug
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-O0 -g)
else()
    add_compile_options(-O2)
endif()

# Windows-specific adjustments
if(WIN32)
    # Clang on Windows may require pthread replacement
    find_package(Threads REQUIRED)
endif()