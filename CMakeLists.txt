cmake_minimum_required(VERSION 3.27)
project(memory_manager C)

set(CMAKE_C_STANDARD 11)

if(NOT CMAKE_C_FLAGS MATCHES "-Wstrict-prototypes")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wstrict-prototypes")
endif(NOT CMAKE_C_FLAGS MATCHES "-Wstrict-prototypes")

if(NOT CMAKE_C_FLAGS MATCHES "-Werror")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror")
endif(NOT CMAKE_C_FLAGS MATCHES "-Werror")

if(NOT CMAKE_C_FLAGS MATCHES "-Wno-expansion-to-defined")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-expansion-to-defined")
endif(NOT CMAKE_C_FLAGS MATCHES "-Wno-expansion-to-defined")

if (CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
    add_definitions(-D__x86_64__)
endif()

option(BUILD_SHARED_LIBS "Build shared libray" ON)

include(CMakePrintHelpers)
cmake_print_variables(CMAKE_BUILD_TYPE)

if (CMAKE_BUILD_TYPE MATCHES "Debug")
    add_definitions(-DDEBUG)
endif()

add_executable(translate_virtual_to_physical_address translate_virtual_to_physical_address.c)

add_library(vm_implicit "")
target_sources(vm_implicit PRIVATE "./vm/mm.c" "./vm/mm.h" "./vm/memlib.c" "./vm/memlib.h")

add_library(vm_explicit "")
target_sources(vm_explicit PRIVATE "./vm/mm.c" "./vm/mm.h" "./vm/memlib.c" "./vm/memlib.h")
target_compile_definitions(vm_explicit PRIVATE "VM_EXPLICIT_LINK")

add_executable(vm_implicit_test "./vm/vm_test.c")
target_link_libraries(vm_implicit_test vm_implicit)

add_executable(vm_explicit_test "./vm/vm_test.c")
target_link_libraries(vm_explicit_test vm_explicit)

add_library(dlmalloc ""
        dlmalloc/dlmalloc.h)
target_sources(dlmalloc PRIVATE "./dlmalloc/dlmalloc.c")