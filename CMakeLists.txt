cmake_minimum_required(VERSION 3.28)
project(memory_manager C)

set(CMAKE_C_STANDARD 11)

if (CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
    add_definitions(-D__x86_64__)
endif()

add_executable(translate_virtual_to_physical_address translate_virtual_to_physical_address.c)
