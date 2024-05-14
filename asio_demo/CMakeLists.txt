cmake_minimum_required(VERSION 3.7)
project(asio_demo C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 11)

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

add_library(asio_demo INTERFACE)

find_package(Threads REQUIRED)
find_package(Boost 1.53.0 COMPONENTS system REQUIRED)
target_include_directories(asio_demo INTERFACE ${Boost_INCLUDE_DIRS})
target_link_libraries(asio_demo INTERFACE Boost::boost Boost::system)

add_executable(tuttimer1 "./tutorial/tuttimer1.cpp")
target_link_libraries(tuttimer1 asio_demo)

add_executable(tuttimer2 "./tutorial/tuttimer2.cpp")
target_link_libraries(tuttimer2 asio_demo)

add_executable(tuttimer3 "./tutorial/tuttimer3.cpp")
target_link_libraries(tuttimer3 asio_demo)

add_executable(tuttimer4 "./tutorial/tuttimer4.cpp")
target_link_libraries(tuttimer4 asio_demo)