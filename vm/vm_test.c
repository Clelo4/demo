#include <assert.h>
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
  long *b = mm_malloc(sizeof(long));
  *a = 12;
  *b = 24;
  mm_free(a);
  mm_free(b);
  // Check if memory is freed properly
}

int get_random_number(int min, int max) {
  return rand() % (max - min + 1) + min;
}

void malloc_large_size_allocation(void) {
  int cout = 10240;
  void **ptr_list = (void **) malloc((cout + 2) * sizeof(void *));
  for (int i = 1; i <= cout; i++) {
    int num = i + get_random_number(1, cout);
    ptr_list[i] = mm_malloc(num);
    assert(ptr_list[i] != ptr_list[i - 1]);
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

  int* start_ptr = mm_malloc(sizeof(int));
  mm_free(start_ptr);
  malloc_multiple_allocations_and_free();
  for (int i = 1; i < 100; i++) {
    int* start = mm_malloc(sizeof(int) * i * i);
    malloc_multiple_allocations_and_free();
    test_mm_malloc();
    malloc_single_allocation_and_free();
    malloc_multiple_allocations_and_free();
    malloc_large_size_allocation();
    malloc_and_free_in_loop();
    malloc_multiple_allocations_and_free();
    mm_free(start);
  }
  int* end_ptr = mm_malloc(sizeof(int));
  assert(start_ptr == end_ptr);

  return 0;
}
