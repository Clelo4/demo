#include <stdio.h>
#include <stdlib.h>

#include "mm.h"
#include "memlib.h"

void test_mm_malloc(void) {
  int *a = mm_malloc(sizeof(int));
  *a = 12;
  printf("%p: %d\n", a, *a);
  mm_free(a);
}

void more_test_mm_malloc(void) {
  // 增加更多测试用例
}

void malloc_single_allocation_and_free(void) {
  int *a = mm_malloc(sizeof(int));
  *a = 12;
  mm_free(a);
  // Check if memory is freed properly
}

void malloc_multiple_allocations_and_free(void) {
  int *a = mm_malloc(sizeof(int));
  int *b = mm_malloc(sizeof(int));
  *a = 12;
  *b = 24;
  mm_free(a);
  mm_free(b);
  // Check if memory is freed properly
}

void malloc_zero_size_allocation(void) {
  // int *a = mm_malloc(0);
  // Check if NULL is returned
}

void malloc_large_size_allocation(void) {
  void** ptr_list = (void**)malloc(1024 * sizeof(void*));
  int cout = 118;
  for (int i = 1; i <= cout; i++) {
    void *a = mm_malloc(i);
    ptr_list[i] = a;
    // mm_free(ptr_list[i]);
    // Check if memory is allocated and freed properly
  }
  for (int i = 1; i <= cout; i++) {
    mm_free(ptr_list[i]);
  }
}

void malloc_and_free_in_loop(void) {
  for (int i = 0; i < 100; i++) {
    int *a = mm_malloc(sizeof(int));
    *a = i;
    mm_free(a);
    // Check if memory is freed properly in each iteration
  }
}

void malloc_without_free(void) {
  int *a = mm_malloc(sizeof(int));
  *a = 12;
  // Check if memory is allocated properly without freeing
}

int main(void) {
  mem_init();
  mm_init();

  void* a = malloc(sizeof(int));
  printf("%p\n", a);
  printf("%p\n", a + 2);

  malloc_multiple_allocations_and_free();
  for (int i = 0; i < 5; i++) {
    malloc_multiple_allocations_and_free();
    test_mm_malloc();
    malloc_single_allocation_and_free();
    malloc_multiple_allocations_and_free();
    // malloc_zero_size_allocation();
    malloc_large_size_allocation();
    malloc_and_free_in_loop();
    // malloc_without_free();
    malloc_multiple_allocations_and_free();
  }

  return 0;
}
