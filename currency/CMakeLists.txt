cmake_minimum_required(VERSION 3.7)
project(currency C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (NOT CMAKE_C_FLAGS MATCHES "-Wstrict-prototypes")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wstrict-prototypes")
endif (NOT CMAKE_C_FLAGS MATCHES "-Wstrict-prototypes")

if (NOT CMAKE_C_FLAGS MATCHES "-Werror")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror")
endif (NOT CMAKE_C_FLAGS MATCHES "-Werror")

if (NOT CMAKE_C_FLAGS MATCHES "-Wno-expansion-to-defined")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-expansion-to-defined")
endif (NOT CMAKE_C_FLAGS MATCHES "-Wno-expansion-to-defined")

if (CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
    add_definitions(-D__x86_64__)
endif ()

add_executable(spin_lock_impl_test ./spin_lock/spin_lock_impl_test.cpp)