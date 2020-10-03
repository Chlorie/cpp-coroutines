cmake_minimum_required(VERSION 3.13)

project(cpp-coroutines)

function (add_sources VAR)
    set(sources ${ARGN})
    list(TRANSFORM sources PREPEND "src/")
    set(${VAR} ${sources} PARENT_SCOPE)
endfunction ()

function (config_target TGT)
    # Use C++20
    target_compile_features(${TGT} PUBLIC cxx_std_20)
    set_target_properties(${TGT} PROPERTIES
        CXX_EXTENSIONS OFF
        INTERPROCEDURAL_OPTIMIZATION ON)

    # MSVC settings
    if (MSVC)
        # Force MSVC to use utf-8 encoding regardless of whether the BOM exists
        target_compile_options(${TGT} PUBLIC "/utf-8")
        # /Zc:__cplusplus is needed for MSVC to produce correct value for the __cplusplus macro
        # If this option is not on, __cplusplus will always expands to 199711L
        if (MSVC_VERSION GREATER_EQUAL 1914)
            target_compile_options(${TGT} PUBLIC "/Zc:__cplusplus")
        endif ()
    endif ()

    # Warnings and errors settings
    # Use highest reasonable warning level, and treat warnings as errors
    if (MSVC) # Visual Studio
        if (CMAKE_CXX_FLAGS MATCHES "/W[0-4]") # If default /W3 presents
            string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}") # Change /W3 to /W4
        else ()
            target_compile_options(${TGT} PRIVATE /W4) # Add /W4 directly
        endif ()
        target_compile_options(${TGT} PRIVATE /WX) # Treat warnings as errors
        # Treat all header files specified by angle brackets to be system headers, and ignore all those warnings
        target_compile_options(${TGT} PRIVATE 
            /experimental:external /external:W0 /external:anglebrackets)
    else () # Not Visual Studio, assuming gcc or clang
        target_compile_options(${TGT} PRIVATE
            -Wall -Wextra -pedantic -Wundef -Wconversion -Wzero-as-null-pointer-constant -Werror)
    endif ()
endfunction ()

add_executable(coro)
add_sources(CORO_SOURCES
    "main.cpp"
    "concepts.h"
    "generator.h"
    "schedule.h"
    "spawn.h"
    "static_thread_pool.h"
    "static_thread_pool.cpp"
    "task.h")
target_sources(coro PRIVATE ${CORO_SOURCES})
config_target(coro)
