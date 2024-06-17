cmake_minimum_required(VERSION 3.7)
project(syscall C ASM)

# 指定语言
enable_language(C CXX ASM)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# 添加可执行文件或库
add_executable(syscall_asm "./syscall_asm.s")
# 添加可执行文件或库
add_executable(syscall_c "./syscall_asm.c")

set_source_files_properties(syscall_asm.s PROPERTIES COMPILE_FLAGS "-fPIE")
target_compile_options(syscall_asm PRIVATE -fPIE)