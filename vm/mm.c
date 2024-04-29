#include <stddef.h>

#include "memlib.h"

#define WSIZE 4
#define DSIZE 8
#define CHUNSIZE (1 << 12)
#define MIN_BLOCK_SIZE (4 * WSIZE)

#define MAX(x, y) ((x) > (y) ? (x) : (y))

// Pack a size and allocated bit into a word
#define PACK(size, alloc) ((size) | (alloc))

// Read and write a word at address p
#define GET(p) (*(unsigned int*)(p))
#define PUT(p, val) (*(unsigned int*)(p) = (val))

// Read the size and allocated fields from address p
#define GET_SIZE(p) (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)

// Given block ptr bp, compute address of its header and footer
#define HDRP(bp) ((char*)(bp) - WSIZE)
#define FTRP(bp) ((char*)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

// Given block ptr bp, compute address of next and previous blocks
#define NEXT_BLKP(bp) ((char*)(bp) + GET_SIZE(((char*)(bp) - WSIZE)))
#define PREV_BLKP(bp) ((char*)(bp) - GET_SIZE(((char*)(bp) - DSIZE)))

static char* heap_listp = NULL;

static void* find_fit(size_t asize);

static void place(void* bp, size_t asize);

static void* coalesce(void* bp);

static void* extend_heap(size_t words) {
  char* bp;
  size_t size;

  // Allocate an even number of words to maintain alignment
  size = (words % 2) ? (words + 1) * WSIZE : words * WSIZE;
  if ((long)(bp = mem_sbrk(size)) == -1) return NULL;

  // Initialize free block header/footer and the epilogue header
  PUT(HDRP(bp), PACK(size, 0));          // Free block header
  PUT(FTRP(bp), PACK(size, 0));          // Free block header
  PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));  // New epilogue header

  // Coalesce if the previous block was free
  return coalesce(bp);
}

static void* coalesce(void* bp) {
  size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
  size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
  size_t size = GET_SIZE(HDRP(bp));

  if (prev_alloc && next_alloc) {
    return bp;
  }

  else if (prev_alloc && !next_alloc) {
    size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
  }

  else if (!prev_alloc && next_alloc) {
    size += GET_SIZE(HDRP(PREV_BLKP(bp)));
    PUT(FTRP(bp), PACK(size, 0));
    PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
    bp = PREV_BLKP(bp);
  }

  else {
    size += GET_SIZE(HDRP(PREV_BLKP(bp))) + GET_SIZE(FTRP(NEXT_BLKP(bp)));
    PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
    PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
    bp = PREV_BLKP(bp);
  }

  return bp;
}

int mm_init(void) {
  if ((heap_listp = mem_sbrk(MIN_BLOCK_SIZE)) == (void*)-1) return -1;
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
  if (extend_heap(CHUNSIZE / WSIZE) == NULL) return -1;
  return 0;
}

void* mm_malloc(size_t size) {
  // Adjusted block size
  size_t asize;
  // Amount to extend heap if no fit
  size_t extendsize;
  char* bp;

  // Ignore spurious requests
  if (size == 0) return NULL;

  if (size <= DSIZE)
    asize = 2 * DSIZE;
  else
    asize = DSIZE * ((size + DSIZE + DSIZE - 1) / DSIZE);

  // Search the free list for a fit
  if ((bp = find_fit(asize)) != NULL) {
    place(bp, asize);
    return bp;
  }

  // Not fit found. Get more memory and place the block
  extendsize = MAX(asize, CHUNSIZE);
  if ((bp = extend_heap(extendsize / WSIZE)) == NULL) return NULL;

  place(bp, asize);
  return bp;
}

void mm_free(void* bp) {
  size_t size = GET_SIZE(HDRP(bp));
  PUT(HDRP(bp), PACK(size, 0));
  PUT(FTRP(bp), PACK(size, 0));

  coalesce(bp);
}

static void* find_fit(size_t asize) {
  void* current_bp = heap_listp;

  void* next_bp = NULL;
  while ((next_bp = NEXT_BLKP(current_bp)) &&
         !(GET_SIZE(next_bp) == 0 && GET_ALLOC(next_bp) == 0)) {
    if (GET_SIZE(next_bp) >= asize && GET_ALLOC(next_bp) == 0) {
      return next_bp;
    }
    current_bp = next_bp;
  }

  return NULL;
}

// Place bp in the first empty block. Split the empty block only if the size of
// the left block is larger than MIN_BLOCK_SIZE.
static void place(void* bp, size_t asize) {
  size_t empty_size = GET_SIZE(HDRP(bp));

  PUT(HDRP(bp), PACK(asize, 1));
  PUT(FTRP(bp), PACK(asize, 1));

  size_t remain_size = empty_size - asize;
  if (remain_size >= MIN_BLOCK_SIZE) {
    PUT(HDRP(NEXT_BLKP(bp)), PACK(remain_size, 0));
    PUT(FTRP(NEXT_BLKP(bp)), PACK(remain_size, 0));
  }
}
