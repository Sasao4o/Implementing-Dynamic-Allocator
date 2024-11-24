/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 * 
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "FC",
    /* First member's full name */
    "Mostafa Mohamed",
    /* First member's email address */
    "sasa1x2001@gmail.com",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)


#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

/* Basic constants and macros */
#define WSIZE 4        /* Word and header/footer size (bytes) */
#define DSIZE 8        /* Double word size (bytes) */
#define CHUNKSIZE (1 << 12) /* Extend heap by this amount (bytes) */

/* Maximum of two values */
#define MAX(x, y) ((x) > (y) ? (x) : (y))

/* Pack a size and allocated bit into a word */
#define PACK(size, alloc) ((size) | (alloc))

/* Read and write a word at address p */
#define GET(p) (*(unsigned int *)(p))
#define PUT(p, val) (*(unsigned int *)(p) = (val))

/* Read the size and allocated fields from address p */
#define GET_SIZE(p) (GET(p) & ~0x7)  /* Clear the last 3 bits to get the block size */
#define GET_ALLOC(p) (GET(p) & 0x1)  /* Extract the allocation bit */
 
/* Given block pointer bp, compute address of its header and footer */
#define HDRP(bp) ((char *)(bp) - WSIZE)
#define FTRP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)
#define SUCC(bp) ((char *)(bp) + WSIZE)
#define PRE(bp) ((char *)(bp))
#define GET_PRE(bp) (*(int*)((char *)(bp)))
#define GET_SUCC(bp) (*(int*)(SUCC(bp)))
/* Given block pointer bp, compute address of next and previous blocks */
#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE)))
#define PREV_BLKP(bp) ((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE)))


//Global Variables
static char *heap_list;
/* 
 * mm_init - initialize the malloc package.
 */
static void *coalesce(void *bp) {
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp))); // Allocation status of the previous block
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp))); // Allocation status of the next block
    size_t size = GET_SIZE(HDRP(bp));                  // Size of the current block

    // Case 1: Both previous and next blocks are allocated
    if (prev_alloc && next_alloc) {
        return bp;
    }

    // Case 2: Previous block is allocated, next block is free
    else if (prev_alloc && !next_alloc) {
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));        // Add next block's size to the current block
        PUT(HDRP(bp), PACK(size, 0));                // Update header of the current block
        PUT(FTRP(bp), PACK(size, 0));                // Update footer of the current block
    }

    // Case 3: Previous block is free, next block is allocated
    else if (!prev_alloc && next_alloc) {
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));       // Add previous block's size to the current block
        PUT(FTRP(bp), PACK(size, 0));                // Update footer of the current block
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));     // Update header of the previous block
        bp = PREV_BLKP(bp);                          // Update bp to point to the previous block
    }

    // Case 4: Both previous and next blocks are free
    else {
        size += GET_SIZE(HDRP(PREV_BLKP(bp))) +
                GET_SIZE(FTRP(NEXT_BLKP(bp)));       // Add sizes of previous and next blocks
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));     // Update header of the previous block
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));     // Update footer of the next block
        bp = PREV_BLKP(bp);                          // Update bp to point to the previous block
    }

    return bp;
}


static void* insert_in_freelist(char *bp) {
    int rootSucessor = GET_SUCC(heap_list);
    if (rootSucessor == -1) {
        //First Insertion
        PUT(SUCC(heap_list), bp - heap_list);
        // printf("Putting in Root Sucessor %d\n" , bp - heap_list);
    } else {
        //Insertion Policy is LIFO
        unsigned int oldSucessor = GET(SUCC(heap_list));
        PUT(SUCC(heap_list), bp - heap_list);
        PUT(SUCC(bp), oldSucessor);
        PUT(PRE(heap_list + oldSucessor), SUCC(heap_list));
        // printf("Putting in Root Sucessor %d\n" , bp - heap_list);
    }

    return NULL;
}

static void *extend_heap(size_t words) {
 
    char *bp;       // Pointer to the new block
    size_t size;    // Size to extend the heap

    // Allocate an even number of words to maintain alignment
    size = (words % 2) ? (words + 1) * WSIZE : words * WSIZE;
    printf ("Extending Size with %d \n", size);
        // printf("Extending Heap with %d\n", size);
    if ((long)(bp = mem_sbrk(size)) == -1) {
        printf("Failed inside extend_heap\n");
        return NULL; // Return NULL if heap extension fails
    }

    // Initialize the free block header/footer and the epilogue header
    PUT(HDRP(bp), PACK(size, 0));                // Free block header
    PUT(FTRP(bp), PACK(size, 0));                // Free block footer
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));        // New epilogue header
    insert_in_freelist(bp);
    // Coalesce if the previous block was free
    // return coalesce(bp);
    return NULL;
}

int mm_init(void)
{   
    if ((heap_list = mem_sbrk(6 * WSIZE)) == (void*)-1) return -1;
 
    
    PUT(heap_list, 0); //Padding
    PUT(heap_list + WSIZE , PACK(2 * DSIZE, 1)); //Header
    PUT(heap_list + (4 * WSIZE) , PACK(2 * DSIZE, 1)); //Footer
    PUT(heap_list + (2 *WSIZE) , -1); //Predecsor
    PUT(heap_list + (3 *WSIZE) , -1); //Sucessor
    PUT(heap_list + (5 *WSIZE) , PACK(0, 1)); //Epilogue Header
    char *x = heap_list;
    heap_list += 2 * WSIZE;
 
    // printf("precssor is%d\n", GET_PRE(heap_list));
    // printf("succ is%d\n", *(int*)((char*)heap_list + (WSIZE)));
    //TODO check error
  
    extend_heap(CHUNKSIZE/WSIZE);
    //TODO adjust free linked list

    // PUT(SUCC(heap_list), NEXT_BLKP(heap_list) + WSIZE -  x);
    // printf("Suffix Offset%d\n",NEXT_BLKP(heap_list) - heap_list);
    return 0;
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
// void *mm_mallocV1(size_t size)
// {
//     int newsize = ALIGN(size + SIZE_T_SIZE);
//     void *p = mem_sbrk(newsize);
//     if (p == (void *)-1)
// 	return NULL;
//     else {
//         *(size_t *)p = size;
//         return (void *)((char *)p + SIZE_T_SIZE);
//     }
// }
 
 static void* find_fitOpitimized(size_t asize) 
{
 
}

static void* find_fit(size_t asize) 
{
    /* First-fit search */
    void* bp;
    printf("Required size is %d\n", asize);
    for (bp = heap_list; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp)) {
        printf("Size of Candidate is %d\n", GET_SIZE(HDRP(bp)));
        if (!GET_ALLOC(HDRP(bp)) && (asize <= GET_SIZE(HDRP(bp)))) {

            return bp;
        }
    }
    printf("Not Found in find_fit\n");
    return NULL; /* No fit */
}
void *mm_malloc(size_t size)
{   
    if (size < 8) size = 8; //minumum requirments because of predcssor and sucessor
    int newSize = ALIGN(size);
 
    int totalSize = ALIGN(newSize + 8);
    //|Header|Payload|Footer| (Payload become metadata in freeblocks)
    //We Align the Payload (size) and put the 8 of footer and header and align the whole block
   
    void *bp = find_fit(totalSize);
    
    if (bp == NULL) {
        fprintf(stderr, "Error: No suitable block found for size %zu\n", totalSize);
        extend_heap(totalSize/WSIZE);
        bp = find_fit(totalSize);
        if (bp == NULL) {{
            printf("Unexpected Error in mm_malloc\n");
            return NULL;
        }}
    } 
    size_t bp_size = GET_SIZE(HDRP(bp));
    assert(totalSize <= bp_size);
    //Splitting Or No?
    PUT(HDRP(bp), PACK(bp_size, 1));
    PUT(FTRP(bp), PACK(bp_size, 1));
    return bp;
 
}

/*
 * mm_free - Freeing a block does nothing.
 */
// void mm_freeV1(void *ptr)
// {
// }

void mm_free(void *ptr)
{
    
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    void *oldptr = ptr;
    void *newptr;
    size_t copySize;
    
    newptr = mm_malloc(size);
    if (newptr == NULL)
      return NULL;
    copySize = *(size_t *)((char *)oldptr - SIZE_T_SIZE);
    if (size < copySize)
      copySize = size;
    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    return newptr;
}














