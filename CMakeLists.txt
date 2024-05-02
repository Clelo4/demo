cmake_minimum_required(VERSION 3.28)
project(memory_manager C)

set(CMAKE_C_STANDARD 11)

if(NOT CMAKE_C_FLAGS MATCHES "-Wstrict-prototypes")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wstrict-prototypes")
endif(NOT CMAKE_C_FLAGS MATCHES "-Wstrict-prototypes")

if(NOT CMAKE_C_FLAGS MATCHES "-Werror")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror")
endif(NOT CMAKE_C_FLAGS MATCHES "-Werror")

if (CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
    add_definitions(-D__x86_64__)
endif()

option(BUILD_SHARED_LIBS "Build shared libray" ON)

add_executable(translate_virtual_to_physical_address translate_virtual_to_physical_address.c)

add_library(vm "")
target_sources(vm PRIVATE "./vm/mm.c" "./vm/mm.h" "./vm/memlib.c" "./vm/memlib.h")
target_compile_definitions(vm PRIVATE "VM_EXPLICIT_LINK")

add_executable(vm_test "./vm/vm_test.c")
target_link_libraries(vm_test vm)

add_library(dlmalloc ""
        dlmalloc/dlmalloc.h)
target_sources(dlmalloc PRIVATE "./dlmalloc/dlmalloc.c")