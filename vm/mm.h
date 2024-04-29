#ifndef MM_H
#define MM_H
#include <stddef.h>

int mm_init(void);
void *mm_malloc (size_t size);
void mm_free (void *ptr);

#endif