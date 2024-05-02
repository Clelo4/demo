#include <stddef.h>
#include <stdint.h>

#include "memlib.h"

#define WSIZE (sizeof(uint32_t))
#define DSIZE (sizeof(void*))
#define CHUNSIZE (1 << 12)
#define CHUNDSIZE 513
#define MIN_BLOCK_SIZE (4 * WSIZE)
#define MIN_BLOCK_DSIZE 3

#define MAX(x, y) ((x) > (y) ? (x) : (y))

// Pack a size and allocated bit into a word
#define PACK(size, alloc) ((size) | (alloc))

// Read and write a word at address p
#define GET(p) (*(uint32_t*)(p))
#define PUT(p, val) (*(uint32_t*)(p) = (val))

// Read the size and allocated fields from address p
#define GET_SIZE(p) (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)

// Given block ptr bp, compute address of its header and footer
#define HDRP(bp) ((char*)(bp) - WSIZE)
#define FTRP(bp) ((char*)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

// Given block ptr bp, compute address of next and previous blocks
#define NEXT_BLKP(bp) ((char*)(bp) + GET_SIZE(((char*)(bp) - WSIZE)))
#define PREV_BLKP(bp) ((char*)(bp) - GET_SIZE(((char*)(bp) - DSIZE)))

// Get next link block pointer
#define NEXT_BLOCK_PTR(bp) (*((void**)((void*)bp + sizeof(void*))))
// Get pre link block pointer
#define PRE_BLOCK_PTR(bp) (*(void**)bp)

static char *heap_listp = NULL;

static void *implicit_find_fit(size_t asize);

static void implicit_place(void *bp, size_t asize);

static void *implicit_extend_heap(size_t words);

static void *implicit_coalesce(void *bp);

// Explicit link
static char* link_list;

static void *explicit_find_fit(size_t adsize);

inline int find_fit_index(size_t adsize);

static void explicit_place(void *bp, size_t adsize);

static void *explicit_extend_heap(size_t words);

static void *explicit_coalesce(void *bp, int is_free);

static void explicit_remove_block(void *bp);

void explicit_insert_to_list(void *bp, int index);

inline static int implicit_mm_init(void) {
  if ((heap_listp = mem_sbrk(MIN_BLOCK_SIZE)) == (void *) -1) return -1;
  // Alignment padding
  PUT(heap_listp, 0);
  // Prologue header
  PUT(heap_listp + (1 * WSIZE), PACK(DSIZE, 1));
  // Prologue footer
  PUT(heap_listp + (2 * WSIZE), PACK(DSIZE, 1));
  // Epilogue header;
  PUT(heap_listp + (3 * WSIZE), PACK(0, 1));

  heap_listp += (2 * WSIZE);

  // Extend the empty headp with a free block of CHUNSIZE bytes
  if (implicit_extend_heap(CHUNSIZE / WSIZE) == NULL) return -1;
  return 0;
}

inline static void *implicit_mm_malloc(size_t size) {
  // Adjusted block size
  size_t asize;
  // Amount to extend heap if no fit
  size_t extendsize;
  void *bp;

  // Ignore spurious requests
  if (size == 0) return NULL;

  if (size <= DSIZE)
    asize = 2 * DSIZE;
  else
    // Add more one DSIZE for header and footer.
    asize = DSIZE * ((size + DSIZE + DSIZE - 1) / DSIZE);

  // Search the free list for a fit
  if ((bp = implicit_find_fit(asize)) != NULL) {
    implicit_place(bp, asize);
    return bp;
  }

  // Not fit found. Get more memory and place the block
  extendsize = MAX(asize, CHUNSIZE);
  if ((bp = implicit_extend_heap(extendsize / WSIZE)) == NULL) return NULL;

  implicit_place(bp, asize);
  return bp;
}

inline static void implicit_mm_free(void *bp) {
  size_t size = GET_SIZE(HDRP(bp));
  PUT(HDRP(bp), PACK(size, 0));
  PUT(FTRP(bp), PACK(size, 0));

  implicit_coalesce(bp);
}

void *implicit_find_fit(size_t asize) {
  void *current_bp = heap_listp;

  void *next_bp = NULL;
  while ((next_bp = NEXT_BLKP(current_bp)) &&
    !(GET_SIZE(HDRP(next_bp)) == 0 && GET_ALLOC(next_bp) == 0)) {
    if (GET_SIZE(HDRP(next_bp)) >= asize && GET_ALLOC(next_bp) == 0) {
      return next_bp;
    }
    current_bp = next_bp;
  }

  return NULL;
}

// Place bp in the first empty block. Split the empty block only if the size of
// the left block is larger than MIN_BLOCK_SIZE.
void implicit_place(void *bp, size_t asize) {
  size_t total_size = GET_SIZE(HDRP(bp));
  size_t remain_size = total_size - asize;

  // 默认使用total_size大小，如果剩余remain_size大于MIN_BLOCK_SIZE，则使用asize
  if (remain_size >= MIN_BLOCK_SIZE) {
    PUT(HDRP(bp), PACK(asize, 1));
    PUT(FTRP(bp), PACK(asize, 1));

    PUT(HDRP(NEXT_BLKP(bp)), PACK(remain_size, 0));
    PUT(FTRP(NEXT_BLKP(bp)), PACK(remain_size, 0));
  }
}

void *implicit_extend_heap(size_t words) {
  char *bp;
  size_t size;

  // Allocate an even number of words to maintain alignment
  size = (words % 2) ? (words + 1) * WSIZE : words * WSIZE;
  if ((long) (bp = mem_sbrk(size)) == -1) return NULL;

  // Initialize free block header/footer and the epilogue header
  PUT(HDRP(bp), PACK(size, 0)); // Free block header
  PUT(FTRP(bp), PACK(size, 0)); // Free block header
  PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1)); // New epilogue header

  // Coalesce if the previous block was free
  return implicit_coalesce(bp);
}

void *implicit_coalesce(void *bp) {
  size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
  size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
  size_t size = GET_SIZE(HDRP(bp));

  if (prev_alloc && next_alloc) {
    return bp;
  } else if (prev_alloc && !next_alloc) {
    size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
  } else if (!prev_alloc && next_alloc) {
    size += GET_SIZE(HDRP(PREV_BLKP(bp)));
    PUT(FTRP(bp), PACK(size, 0));
    PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
    bp = PREV_BLKP(bp);
  } else {
    size += GET_SIZE(HDRP(PREV_BLKP(bp))) + GET_SIZE(FTRP(NEXT_BLKP(bp)));
    PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
    PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
    bp = PREV_BLKP(bp);
  }

  return bp;
}

const int kHeadBlockSize = WSIZE + sizeof(void*) * 2 + WSIZE;

#define GET_HEAD_BP(index) ((link_list) + ((index) * kHeadBlockSize) + WSIZE)

inline static int explicit_mm_init(void) {
  // 启发式策略：尽量维持少量的大空闲块，128个小块，3个大块
  int total_list_size = (128 + 3) * kHeadBlockSize;
  // 对total_list_size向上取最近的8倍整数
  total_list_size = DSIZE * ((total_list_size + DSIZE - 1) / DSIZE);

  if ((link_list = mem_sbrk(total_list_size)) == (void *) -1) return -1;

  // 初始化小块：1-128大小的小块
  for (int i = 0; i < 131; i++) {
    void* bp = GET_HEAD_BP(i);
    PRE_BLOCK_PTR(bp) = NULL;
    NEXT_BLOCK_PTR(bp) = NULL;
    PUT(HDRP(bp), PACK(kHeadBlockSize, 1));
    PUT(FTRP(bp), PACK(kHeadBlockSize, 1));

  }

  void *head;
  if ((head = mem_sbrk(2 * DSIZE)) == (void *) -1) return -1;
  // Alignment padding
  PUT(head, 0);
  // Prologue header
  PUT(head + (1 * WSIZE), PACK(DSIZE, 1));
  // Prologue footer
  PUT(head + (2 * WSIZE), PACK(DSIZE, 1));
  // Epilogue header;
  PUT(head + (3 * WSIZE), PACK(0, 1));

  return 0;
}

inline static void *explicit_mm_malloc(size_t size) {
  // Adjusted block dsize
  size_t adsize;

  // 不允许分配大小为0的堆内存
  if (size == 0) return NULL;

  if (size <= 2 * DSIZE)
    adsize = MIN_BLOCK_DSIZE;
  else
    // Add more one DSIZE for header and footer.
    adsize = (size + DSIZE + DSIZE - 1) / DSIZE;

  void *bp;
  // Search the free list for a fit
  if ((bp = explicit_find_fit(adsize)) != NULL) {
    explicit_place(bp, adsize);
    return bp;
  }

  // Not fit found. Get more memory and place the block
  size_t extenddsize = MAX(adsize, CHUNDSIZE);
  if ((bp = explicit_extend_heap(extenddsize)) == NULL) return NULL;

  explicit_place(bp, adsize);
  return bp;
}

inline static void explicit_mm_free(void *bp) {
  size_t size = GET_SIZE(HDRP(bp));

  PUT(HDRP(bp), PACK(size, 0));
  PUT(FTRP(bp), PACK(size, 0));

  explicit_coalesce(bp, 0);
}

void *explicit_find_fit(size_t adsize) {
  int start_i = 0;

  if (adsize <= 128) {
    start_i = adsize - 1;
  } else {
    start_i = 129;
  }

  for (int i = start_i; i <= 130; i++) {
    void* head = GET_HEAD_BP(i);
    void* cur = NEXT_BLOCK_PTR(head);
    while (cur != NULL) {
      size_t size = GET_SIZE(HDRP(cur));
      // 首次适配
      if (size >= adsize * DSIZE) return cur;
      cur = NEXT_BLOCK_PTR(cur);
    }
  }

  // 未找到
  return NULL;
}

int find_fit_index(size_t adsize) {
  // 将新分割出来的块放在link_list中合适的位置
  int fit_index;
  if (adsize <= 128) {
    fit_index = adsize - 1;
  } else if (adsize <= 256) {
    fit_index = 128;
  } else if (adsize <= 512) {
    fit_index = 129;
  } else {
    fit_index = 130;
  }
  return fit_index;
}

void explicit_place(void *bp, size_t adsize) {
  size_t total_size = GET_SIZE(HDRP(bp));
  size_t remain_size = total_size - adsize * DSIZE;

  explicit_remove_block(bp);

  // 分割当前块
  if (remain_size >= MIN_BLOCK_DSIZE * DSIZE) {
    PUT(HDRP(bp), PACK(adsize * DSIZE, 1));
    PUT(FTRP(bp), PACK(adsize * DSIZE, 1));

    void *next = NEXT_BLKP(bp);
    PUT(HDRP(next), PACK(remain_size, 0));
    PUT(FTRP(next), PACK(remain_size, 0));

    // 将新分割出来的空闲块，放在link_list中合适的位置
    int next_fit_index = find_fit_index(remain_size / DSIZE);
    explicit_insert_to_list(next, next_fit_index);
  }
}

void *explicit_extend_heap(size_t extend_dsize) {
  void *bp;
  if ((long) (bp = mem_sbrk(extend_dsize * DSIZE)) == -1) return NULL;

  // Initialize free block header/footer and the epilogue header
  PUT(HDRP(bp), PACK(extend_dsize * DSIZE, 0)); // Free block header
  PUT(FTRP(bp), PACK(extend_dsize * DSIZE, 0)); // Free block footer
  PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1)); // New epilogue header;

  int fit_index = find_fit_index(extend_dsize);

  void* footer = FTRP(bp);
  explicit_insert_to_list(bp, fit_index);

  return explicit_coalesce(bp, 1);
}

void *explicit_coalesce(void *bp, int is_free) {
  size_t size = GET_SIZE(HDRP(bp));
  size_t cur_index = find_fit_index(size / DSIZE);

  size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
  size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));

  if (prev_alloc && next_alloc) {
    return bp;
  } else if (prev_alloc && !next_alloc) {
    void *next = NEXT_BLKP(bp);
    size += GET_SIZE(HDRP(next));

    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    int new_index = find_fit_index(size / DSIZE);

    // 提取next
    explicit_remove_block(next);

    if (is_free) {
      // 提取bp
      explicit_remove_block(bp);
    }

    explicit_insert_to_list(bp, new_index);
    return bp;
  } else if (!prev_alloc && next_alloc) {
    void *pre = PREV_BLKP(bp);
    size += GET_SIZE(HDRP(PREV_BLKP(bp)));

    PUT(FTRP(bp), PACK(size, 0));
    PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
    int new_index = find_fit_index(size / DSIZE);

    // 提取pre
    explicit_remove_block(pre);

    if (is_free) {
      // 提取bp
      explicit_remove_block(bp);
    }

    explicit_insert_to_list(pre, new_index);

    return pre;
  } else {
    void *pre = PREV_BLKP(bp);
    void *next = NEXT_BLKP(bp);
    size += GET_SIZE(HDRP(pre)) + GET_SIZE(FTRP(next));
    PUT(HDRP(pre), PACK(size, 0));
    PUT(FTRP(pre), PACK(size, 0));

    int new_index = find_fit_index(size / DSIZE);

    // 提取pre
    explicit_remove_block(pre);
    // 提取next
    explicit_remove_block(next);

    if (is_free) {
      // 提取bp
      explicit_remove_block(bp);
    }

    explicit_insert_to_list(pre, new_index);

    return pre;
  }
}

static void explicit_remove_block(void *bp) {
  if (PRE_BLOCK_PTR(bp) != NULL)
    NEXT_BLOCK_PTR(PRE_BLOCK_PTR(bp)) = NEXT_BLOCK_PTR(bp);
  if (NEXT_BLOCK_PTR(bp) != NULL)
    PRE_BLOCK_PTR(NEXT_BLOCK_PTR(bp)) = PRE_BLOCK_PTR(bp);
}

void explicit_insert_to_list(void *bp, int index) {
  void* head = GET_HEAD_BP(index);
  NEXT_BLOCK_PTR(bp) = NEXT_BLOCK_PTR(head);
  PRE_BLOCK_PTR(bp) = head;
  if (NEXT_BLOCK_PTR(head) != NULL)
    PRE_BLOCK_PTR(NEXT_BLOCK_PTR(head)) = bp;
  NEXT_BLOCK_PTR(head) = bp;
}

int mm_init(void) {
#ifdef VM_EXPLICIT_LINK
  return explicit_mm_init();
#else
  return implicit_mm_init();
#endif // VM_EXPLICIT_LINK
}

void *mm_malloc(size_t size) {
#ifdef VM_EXPLICIT_LINK
  return explicit_mm_malloc(size);
#else
  return implicit_mm_malloc(size);
#endif // VM_EXPLICIT_LINK
}

void mm_free(void *ptr) {
#ifdef VM_EXPLICIT_LINK
  explicit_mm_free(ptr);
#else
  implicit_mm_free(ptr);
#endif // VM_EXPLICIT_LINK
}
