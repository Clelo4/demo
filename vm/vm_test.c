#include <stdio.h>
#include "mm.h"
#include "memlib.h"

int main(void)
{
  mem_init();
  mm_init();

  int* a = mm_malloc(sizeof(int));
  *a = 12;

  printf("%p: %d\n", a, *a);
  mm_free(a);

  int* b = mm_malloc(sizeof(int));
  *b = 101;
  printf("%p: %d\n", b, *b);
  mm_free(b);
  return 0;
}