#ifndef MM_H
#define MM_H
#include <stddef.h>

// TODO: 
// 一种改进空间利用率的方式：已使用块不保留footer，未使用块依旧保留footer，用下一个块的header的多余位标识

int mm_init(void);
void *mm_malloc(size_t size);
void mm_free(void *ptr);

#endif
