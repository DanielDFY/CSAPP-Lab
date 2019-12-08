# Malloc Lab

> Answer with detailed comments

```c
/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)

/* basic constants and macros */
#define WSIZE 4     // word and header/footer size (bytes)
#define DSIZE 8     // double word size (bytes)

/* extend heap by this amount (bytes) */
#define INITCHUNKSIZE (1 << 6)
#define CHUNKSIZE (1 << 12)

/* array size of segregated free list */
#define LISTSIZE 16

#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define MIN(x, y) ((x) < (y) ? (x) : (y))

/* pack a size and allocated bit into a word */
#define PACK(size, alloc) ((size) | (alloc))

/* read and write a word at address p */
#define GET(p) (*(unsigned int *)(p))
#define PUT(p, val) (*(unsigned int *)(p) = (val))

#define SET_PTR(p, ptr) (*(unsigned int *)(p) = (unsigned int)(ptr))

/* read the size and allocated fields from address p */
#define GET_SIZE(p)  (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)

/* given block ptr bp, compute address of its header and footer */
#define HDRP(ptr) ((char *)(ptr) - WSIZE)
#define FTRP(ptr) ((char *)(ptr) + GET_SIZE(HDRP(ptr)) - DSIZE)

/* given block ptr bp, compute address of next and previous blocks */
#define NEXT_BLKP(ptr) ((char *)(ptr) + GET_SIZE((char *)(ptr) - WSIZE))
#define PREV_BLKP(ptr) ((char *)(ptr) - GET_SIZE((char *)(ptr) - DSIZE))

/* explict linkedlist pointers */
#define PRED_PTR(ptr) ((char *)(ptr))
#define SUCC_PTR(ptr) ((char *)(ptr) + WSIZE)

#define PRED(ptr) (*(char **)(ptr))
#define SUCC(ptr) (*(char **)(SUCC_PTR(ptr)))

/* segregated free list array */
void *segregated_free_lists[LISTSIZE];
/* extend heap */
static void *extend_heap(size_t size);
/* coalesce adjoining free blocks */
static void *coalesce(void *ptr);
/*
 * allocate block of given size from the free block pointed by ptr, 
 * separate remaining space into segregated free list (if larger than 2*DSIZE)
 */
static void *place(void *ptr, size_t size);
/* insert free block pointed by ptr into segregated free list */
static void insert_node(void *ptr, size_t size);
/* delete block pointed by ptr from segregated free list */
static void delete_node(void *ptr);

/* implementation */

static void *extend_heap(size_t size) {
    void *ptr;
    size = ALIGN(size);
    /* use mem_sbrk to extend heap */
    if ((ptr = mem_sbrk(size)) == (void *)-1)
        return NULL;

    /* set the header and footer of the new free block */
    PUT(HDRP(ptr), PACK(size, 0));
    PUT(FTRP(ptr), PACK(size, 0));
    
    /* set epilogue header */
    PUT(HDRP(NEXT_BLKP(ptr)), PACK(0, 1));
    
    /* insert it into segregated free list */
    insert_node(ptr, size);

    /* coalesce if the previous block was free */
    return coalesce(ptr);
}

static void insert_node(void *ptr, size_t size) {
    int index = 0;
    void *search_ptr = NULL;
    void *insert_ptr = NULL;

    /* find the proper list based on given size */
    while ((index < LISTSIZE - 1) && (size > 1)) {
        // ith list: 2^i ~ 2^(i+1), last list: 2^(LISTSIZE - 1) ~ max
        size >>= 1;
        ++index;
    }

    /* find proper insert position to keep ascending order */
    search_ptr = segregated_free_lists[index];
    while ((search_ptr != NULL) && (size > GET_SIZE(HDRP(search_ptr)))) {
        insert_ptr = search_ptr;
        search_ptr = PRED(search_ptr);
    }

    /* 4 cases after searching */
    if (search_ptr != NULL) {
        if (insert_ptr != NULL) {
            /* xxx-> ptr -> xxx: insert in between */
            SET_PTR(PRED_PTR(ptr), search_ptr);
            SET_PTR(SUCC_PTR(search_ptr), ptr);
            SET_PTR(SUCC_PTR(ptr), insert_ptr);
            SET_PTR(PRED_PTR(insert_ptr), ptr);
        } else {
            /* [index] ptr -> xxx: insert in the front of the list with free block behind */
            SET_PTR(PRED_PTR(ptr), search_ptr);
            SET_PTR(SUCC_PTR(search_ptr), ptr);
            SET_PTR(SUCC_PTR(ptr), NULL);
            segregated_free_lists[index] = ptr;
        }
    } else {
        if (insert_ptr != NULL) { 
            /* ->xxx->insert: insert at the end of the list */
            SET_PTR(PRED_PTR(ptr), NULL);
            SET_PTR(SUCC_PTR(ptr), insert_ptr);
            SET_PTR(PRED_PTR(insert_ptr), ptr);
        } else {
            /* [index] insert: insert into an empty list */
            SET_PTR(PRED_PTR(ptr), NULL);
            SET_PTR(SUCC_PTR(ptr), NULL);
            segregated_free_lists[index] = ptr;
        }
    }
}

static void delete_node(void *ptr) {
    int listnumber = 0;
    size_t size = GET_SIZE(HDRP(ptr));

    /* find the proper list based on given size */
    while ((listnumber < LISTSIZE - 1) && (size > 1)) {
        size >>= 1;
        listnumber++;
    }

    /* same 4 cases as above */
    if (PRED(ptr) != NULL) {
        if (SUCC(ptr) != NULL) {
            /* xxx-> ptr -> xxx */
            SET_PTR(SUCC_PTR(PRED(ptr)), SUCC(ptr));
            SET_PTR(PRED_PTR(SUCC(ptr)), PRED(ptr));
        } else {
            /* [index] ptr -> xxx */
            SET_PTR(SUCC_PTR(PRED(ptr)), NULL);
            segregated_free_lists[listnumber] = PRED(ptr);
        }
    } else {
        if (SUCC(ptr) != NULL) {
            /* ->xxx->insert */
            SET_PTR(PRED_PTR(SUCC(ptr)), NULL);
        } else {
            /* [index] insert */
            segregated_free_lists[listnumber] = NULL;
        }
    }
}

static void *coalesce(void *ptr) {
    /* no adjacent free blocks after coalesce */
    _Bool is_prev_alloc = GET_ALLOC(HDRP(PREV_BLKP(ptr)));
    _Bool is_next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(ptr)));
    size_t size = GET_SIZE(HDRP(ptr));

    /* check prev and next block */
    if (is_prev_alloc && is_next_alloc) {
        /* both prev and next block are allocated, no coalesce*/
        return ptr;
    } else if (is_prev_alloc && !is_next_alloc) {
        /* prev block is allocated, next block is free */
        delete_node(ptr);
        delete_node(NEXT_BLKP(ptr));
        size += GET_SIZE(HDRP(NEXT_BLKP(ptr)));
        PUT(HDRP(ptr), PACK(size, 0));
        PUT(FTRP(ptr), PACK(size, 0));
    } else if (!is_prev_alloc && is_next_alloc) {
        /* prev block is free, next block is allocated */
        delete_node(ptr);
        delete_node(PREV_BLKP(ptr));
        size += GET_SIZE(HDRP(PREV_BLKP(ptr)));
        PUT(FTRP(ptr), PACK(size, 0));
        PUT(HDRP(PREV_BLKP(ptr)), PACK(size, 0));
        ptr = PREV_BLKP(ptr);
    } else {
        /* both prev and next block are free */
        delete_node(ptr);
        delete_node(PREV_BLKP(ptr));
        delete_node(NEXT_BLKP(ptr));
        size += GET_SIZE(HDRP(PREV_BLKP(ptr))) + GET_SIZE(HDRP(NEXT_BLKP(ptr)));
        PUT(HDRP(PREV_BLKP(ptr)), PACK(size, 0));
        PUT(FTRP(NEXT_BLKP(ptr)), PACK(size, 0));
        ptr = PREV_BLKP(ptr);
    }

    /* insert combined free block into segregated free list */
    insert_node(ptr, size);

    return ptr;
}

static void *place(void *ptr, size_t size)
{
    size_t ptr_size = GET_SIZE(HDRP(ptr));
    /* remaining size after allocation */
    size_t remainder = ptr_size - size;

    delete_node(ptr);

    /* if remaining size smaller than min block size, do not separate */
    if (remainder < DSIZE * 2) {
        PUT(HDRP(ptr), PACK(ptr_size, 1));
        PUT(FTRP(ptr), PACK(ptr_size, 1));
    }
    /*  
     * to prevent external fragmentation in free process,
     * small blocks and large blocks should be separately placed together
     * the watershed is set based on binary-bal.rep and binary2-bal.rep
     */
    else if (size >= 96) {
        PUT(HDRP(ptr), PACK(remainder, 0));
        PUT(FTRP(ptr), PACK(remainder, 0));
        PUT(HDRP(NEXT_BLKP(ptr)), PACK(size, 1));
        PUT(FTRP(NEXT_BLKP(ptr)), PACK(size, 1));
        insert_node(ptr, remainder);
        return NEXT_BLKP(ptr);
    } else {
        PUT(HDRP(ptr), PACK(size, 1));
        PUT(FTRP(ptr), PACK(size, 1));
        PUT(HDRP(NEXT_BLKP(ptr)), PACK(remainder, 0));
        PUT(FTRP(NEXT_BLKP(ptr)), PACK(remainder, 0));
        insert_node(NEXT_BLKP(ptr), remainder);
    }
    return ptr;
}

/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void) {
    char *heap; 

    /* init segregated free list */
    for (int i = 0; i < LISTSIZE; ++i) {
        segregated_free_lists[i] = NULL;
    }

    /* init heap */
    if ((long)(heap = mem_sbrk(4 * WSIZE)) == -1)
        return -1;

    PUT(heap, 0);                               // alignment padding
    PUT(heap + (1 * WSIZE), PACK(DSIZE, 1));    // prologue header
    PUT(heap + (2 * WSIZE), PACK(DSIZE, 1));    // prologue footer
    PUT(heap + (3 * WSIZE), PACK(0, 1));        // epilogue header

    /* extend the empty heap with a free block of INITCHUNKSIZE */
    if (extend_heap(INITCHUNKSIZE) == NULL)
        return -1;

    return 0;
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size) {
    if (size == 0)
        return NULL;
    /* alignment */
    if (size <= DSIZE) {
        size = 2 * DSIZE;
    } else {
        size = ALIGN(size + DSIZE);
    }


    size_t searchsize = size;
    void *ptr = NULL;

    for (int i = 0; i < LISTSIZE && ptr == NULL; ++i, searchsize >>= 1) {
        /* find proper free block in proper list */
        if (((searchsize <= 1) && (segregated_free_lists[i] != NULL))) {
            ptr = segregated_free_lists[i];
            while ((ptr != NULL) && ((size > GET_SIZE(HDRP(ptr))))) {
                ptr = PRED(ptr);
            }
        }
    }

    /* no proper free block，extend heap */
    if (ptr == NULL) {
        if ((ptr = extend_heap(MAX(size, CHUNKSIZE))) == NULL)
            return NULL;
    }

    /* allocate block */
    ptr = place(ptr, size);

    return ptr;
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr) {
    size_t size = GET_SIZE(HDRP(ptr));

    PUT(HDRP(ptr), PACK(size, 0));
    PUT(FTRP(ptr), PACK(size, 0));

    /* insert it into segregated free list */
    insert_node(ptr, size);

    /* coalesce if the previous block was free */
    coalesce(ptr);
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size) {
    void *new_block = ptr;
    int remainder;

    if (size == 0)
        return NULL;

    if (size <= DSIZE) {
        size = 2 * DSIZE;
    } else {
        size = ALIGN(size + DSIZE);
    }

    if ((remainder = GET_SIZE(HDRP(ptr)) - size) >= 0) {
        /* if new size < old size, return old block */
        return ptr;
    } else if (!GET_ALLOC(HDRP(NEXT_BLKP(ptr))) || !GET_SIZE(HDRP(NEXT_BLKP(ptr)))) {
        /* check if next adjacent block is free or epilogue block to reduce external fragmentation */

        /* if need to extend heap */
        if ((remainder = GET_SIZE(HDRP(ptr)) + GET_SIZE(HDRP(NEXT_BLKP(ptr))) - size) < 0)
        {
            if (extend_heap(MAX(-remainder, CHUNKSIZE)) == NULL)
                return NULL;
            remainder += MAX(-remainder, CHUNKSIZE);
        }

        /* delete free block used above from list and set its new header/footer */
        delete_node(NEXT_BLKP(ptr));
        PUT(HDRP(ptr), PACK(size + remainder, 1));
        PUT(FTRP(ptr), PACK(size + remainder, 1));
    } else {
        /* no available consecutive free blocks, allocate disconsecutive free blocks */
        new_block = mm_malloc(size);
        memcpy(new_block, ptr, GET_SIZE(HDRP(ptr)));
        mm_free(ptr);
    }

    return new_block;
}
```



### Final result

```
Results for mm malloc:
trace  valid  util     ops      secs  Kops
 0       yes   97%    5694  0.000364 15647
 1       yes   99%    5848  0.000263 22210
 2       yes   99%    6648  0.000416 15969
 3       yes   99%    5380  0.000273 19678
 4       yes   99%   14400  0.000440 32712
 5       yes   94%    4800  0.000403 11905
 6       yes   91%    4800  0.000413 11617
 7       yes   95%   12000  0.000364 33003
 8       yes   88%   24000  0.003080  7793
 9       yes   99%   14401  0.000212 67993
10       yes   98%   14401  0.000173 83243
Total          96%  112372  0.006401 17554

Perf index = 58 (util) + 40 (thru) = 98/100
```

