/*
 * Current implementation keeps a simple linked list off all allocated blocks of all sizes.
 * No chunk spaces are allocated of any kind.
 * While not being too space-efficient this allows for maximum flexibility.
 */

#include "arena.h"
#include "list.h"

#include <stdlib.h>
#include <stddef.h>
#include <setjmp.h>     // Only need jmp_buf type for alignment calculation
#include <inttypes.h>
#include <assert.h>

struct arena
{
    list_head blocks;       // List of allocated blocks
    list_head freelist; // List of free blocks
};

/* Alignment of this type is what we need to align allocated pointers to */
typedef union
{
    char c;
    int i;
    long l;
    long long ll;
    double d;
    void* p;
    void (*fptr)();
    jmp_buf jb;
} anytype_t;

typedef struct 
{
    list_head link;
    size_t size;
    _Alignas(anytype_t) char data[0];
} block_t;

#define ALLOC_ALIGNMENT     _Alignof(anytype_t)
#define IS_ALIGNED(ptr)     (((uintptr_t)(ptr) & (ALLOC_ALIGNMENT - 1)) == 0)

//////////////////////////////////////////////////////////////////////////////

arena_t* arena_create(void)
{
    arena_t* arena = (arena_t*) malloc(sizeof(*arena));
    if (!arena) {
        return NULL;
    }

    list_init(&arena->blocks);
    list_init(&arena->freelist);
    return arena;
}

void arena_destroy(arena_t* arena)
{
    if (arena) {
        arena_trim(arena);
        list_head* p = arena->blocks.next;
        while (p != NULL) {
            list_head* next = p->next;
            free(list_entry(p, block_t, link));
            p = next;
        }

        free(arena);
    }
}

void* arena_alloc(arena_t* arena, size_t bytes)
{
    if (!arena) {
        return NULL;
    }

    // Scan freelist for best-fit available block of sufficient size
    block_t* best_fit = NULL;
    list_for_each(arena->freelist, p) {
        block_t* b = list_entry(p, block_t, link);
        if ((b->size >= bytes) && (!best_fit || (b->size < best_fit->size))) {
            best_fit = b;
        }
    }

    if (best_fit) {
        // Do not change best fit block size - we're not reallocating it
        list_remove(&best_fit->link);
        list_insert(&arena->blocks, &best_fit->link);
        return (void*)best_fit->data;
    }

    // Allocate a new block
    block_t* block = (block_t*) calloc(1, sizeof(*block) + bytes);
    if (!block) {
        return NULL;
    }

    block->size = bytes;

    list_insert(&arena->blocks, &block->link);

    return (void*)block->data;
}

void arena_free(arena_t* arena, void* ptr)
{
    if (arena && ptr) {
        block_t* block = containerof(ptr, block_t, data);
        list_remove(&block->link);
        list_insert(&arena->freelist, &block->link);
    }
}

void arena_trim(arena_t* arena)
{
    if (arena) {
        while (!list_empty(&arena->freelist)) {
            list_head* p = arena->freelist.next;
            list_remove(p);
            free(list_entry(p, block_t, link));
        }
    }
}

//////////////////////////////////////////////////////////////////////////////

#if defined(TEST)

#include "test.h"

static void arena_test(void)
{
    arena_t* a = arena_create();
    CU_ASSERT(a != NULL);

    CU_ASSERT_TRUE(list_empty(&a->freelist));
    CU_ASSERT_TRUE(list_empty(&a->blocks));

    void* p = arena_alloc(NULL, 10);
    CU_ASSERT(p == NULL);
    CU_ASSERT_TRUE(list_empty(&a->blocks));

    void* p1 = arena_alloc(a, 10);
    CU_ASSERT(p1 && IS_ALIGNED(p));
    CU_ASSERT_FALSE(list_empty(&a->blocks));

    void* p2 = arena_alloc(a, 0);
    CU_ASSERT(p2 && IS_ALIGNED(p)); 
    CU_ASSERT_FALSE(list_empty(&a->blocks));

    CU_ASSERT_TRUE(list_empty(&a->freelist));
    
    arena_free(a, p1);
    CU_ASSERT_FALSE(list_empty(&a->freelist));

    p = arena_alloc(a, 8);
    CU_ASSERT(p && IS_ALIGNED(p));  
    CU_ASSERT_TRUE(list_empty(&a->freelist));

    arena_trim(a);
    CU_ASSERT_TRUE(list_empty(&a->freelist));

    arena_destroy(a);
}
TEST_ADD(arena_test);

#endif // TEST

//////////////////////////////////////////////////////////////////////////////
