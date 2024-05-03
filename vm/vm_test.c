#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

void malloc_single_allocation_and_free(void) {
  int *a = mm_malloc(sizeof(int));
  *a = 12;
  mm_free(a);
}

void malloc_multiple_allocations_and_free(void) {
  int *a = mm_malloc(sizeof(int));
  long *b = mm_malloc(sizeof(long));
  double* c = mm_malloc(sizeof(double));
  *a = 12;
  *b = 24;
  *c = 12.0;
  mm_free(c);
  mm_free(a);
  char* d = mm_malloc(sizeof(char));
  mm_free(b);
  mm_free(d);
}

int get_random_number(int min, int max) {
  return rand() % (max - min + 1) + min;
}

void malloc_large_size_allocation(void) {
  int cout = 1024;
  void **ptr_list = (void **) malloc((cout + 2) * sizeof(void *));
  for (int i = 1; i <= cout; i++) {
    int num = i + get_random_number(1, cout);
    ptr_list[i] = mm_malloc(num);
    assert(ptr_list[i] != ptr_list[i - 1]);
  }
  for (int i = 1; i <= cout; i++) {
    mm_free(ptr_list[i]);
  }
  free(ptr_list);
}

void malloc_and_free_in_loop(void) {
  for (int i = 0; i < 100; i++) {
    int *a = mm_malloc(sizeof(int));
    *a = i;
    mm_free(a);
  }
}

void random_order_malloc_and_free(void) {
  int cout = 1024;
  void **ptr_list = (void **) malloc(cout * sizeof(void *));
  memset(ptr_list, 0, cout * sizeof(void *));
  for (int i = 1; i < cout; i++) {
    int num = i * 17 + get_random_number(1, cout) * get_random_number(1, 10) + get_random_number(1,  3 * cout);
    ptr_list[i] = mm_malloc(num);
    assert(ptr_list[i] != ptr_list[i - 1]);
  }

  // free in random order
  void **ptr_list_second = (void**) malloc(cout * sizeof(void*));
  memset(ptr_list_second, 0, cout * sizeof(void*));
  for (int i = 1; i < cout; i++) {
    int num = i * 13 + get_random_number(1, cout) * get_random_number(3, 9) + get_random_number(1,  3 * cout);
    ptr_list_second[i] = mm_malloc(num);
    assert(ptr_list_second[i] != ptr_list_second[i - 1]);
    const int free_index = get_random_number(1, cout - 1);
    if (ptr_list[free_index] != NULL && i % 2 == 0) {
      mm_free(ptr_list[free_index]);
      ptr_list[free_index] = NULL;
    }
  }

  for (int i = 1; i < cout; i++) {
    if (ptr_list_second[i] != NULL)
      mm_free(ptr_list_second[i]);
  }

  for (int i = 1; i < cout; i++) {
    if (ptr_list[i] != NULL)
      mm_free(ptr_list[i]);
  }
  free(ptr_list);
}

int main(void) {
  mem_init();
  mm_init();

  int* start_ptr = mm_malloc(sizeof(int));
  mm_free(start_ptr);
  malloc_multiple_allocations_and_free();
  for (int i = 1; i < 256; i++) {
    int* start = mm_malloc(sizeof(int) * i * i);
    malloc_multiple_allocations_and_free();
    malloc_single_allocation_and_free();
    random_order_malloc_and_free();
    random_order_malloc_and_free();
    malloc_multiple_allocations_and_free();
    malloc_large_size_allocation();
    random_order_malloc_and_free();
    malloc_and_free_in_loop();
    malloc_multiple_allocations_and_free();
    random_order_malloc_and_free();
    mm_free(start);
  }
  int* end_ptr = mm_malloc(sizeof(int));
  assert(start_ptr == end_ptr);
  mm_free(end_ptr);

  return 0;
}
