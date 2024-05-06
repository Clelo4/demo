#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "memlib.h"

// 最大堆内存大小
#define MAX_HEAP (1<<28)

static char *mem_heap;
static char *mem_brk;
static char *mem_max_addr;

/**
 * mem_init - Initialize the memory system mode
 */
void mem_init(void) {
  mem_heap = (char *) malloc(MAX_HEAP);
  mem_brk = (char *) mem_heap;
  mem_max_addr = (char *) (mem_heap + MAX_HEAP);
}

/**
 * mem sbrk - Simple model of the sbrk function. Extends the hear ov Incr ovtes
 * and returns the start address ot the new area. this model, the heap cannotbe
 * shrunk.
 */
void *mem_sbrk(size_t incr) {
  char *old_brk = mem_brk;
  if ((incr < 0) || (mem_brk + incr) > mem_max_addr) {
    errno = ENOMEM;
    fprintf(stderr, "Error: mem_sbrk failed. Ran out of memory...\n");
    exit(1);
  }
  mem_brk += incr;
#ifdef DEBUG
  printf("cur mem size: %ld incr: %ld  %p  %p  \n", mem_brk - mem_heap, incr, mem_brk, mem_heap);
#endif

  return (void *) old_brk;
}
