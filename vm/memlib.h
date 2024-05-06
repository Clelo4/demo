#ifndef MEM_LIB_H
#define MEM_LIB_H

#include <stddef.h>

void mem_init(void);
void* mem_sbrk(size_t incr);

#endif