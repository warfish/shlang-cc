/*
 * Current implementation keeps a simple linked list off all allocated blocks of all sizes.
 * No chunk spaces are allocated of any kind.
 * While not being too space-efficient this allows for maximum flexibility.
 */

#include "arena.h"
#include "list.h"

#include <stdlib.h>
#include <stddef.h>
#include <setjmp.h>		// Only need jmp_buf type for alignment calculation
#include <inttypes.h>
#include <assert.h>

struct arena
{
	list_head blocks;		// List of allocated blocks
	list_head freelist;	// List of free blocks
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
	_Alignas(anytype_t) char data[0];
} block_t;

#define ALLOC_ALIGNMENT		_Alignof(anytype_t)
#define IS_ALIGNED(ptr) 	(((uintptr_t)(ptr) & (ALLOC_ALIGNMENT - 1)) == 0)

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
	}
}

void* arena_alloc(arena_t* arena, size_t bytes)
{
	if (!arena) {
		return NULL;
	}

	block_t* block = (block_t*) calloc(1, sizeof(*block) + bytes);
	if (!block) {
		return NULL;
	}

	list_insert(&arena->blocks, &block->link);
	
	void* res = block->data;
	return res;
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
		list_head* p = arena->freelist.next;
		while (p != NULL) {
			list_head* next = p->next;
			free(list_entry(p, block_t, link));
			p = next;
		}
	}
}

//////////////////////////////////////////////////////////////////////////////

#if defined(TEST)

#include "test.h"

static void test_arena(void)
{
	arena_t* a = arena_create();
	CU_ASSERT(a != NULL);

	void* p = NULL;

	p = arena_alloc(NULL, 10);
	CU_ASSERT(p == NULL);

	p = arena_alloc(a, 10);
	CU_ASSERT(p && IS_ALIGNED(p));

	p = arena_alloc(a, 0);
	CU_ASSERT(p && IS_ALIGNED(p));	

	CU_ASSERT(list_empty(&a->freelist));
	arena_destroy(a);
}
TEST_ADD(test_arena);

#endif // TEST

//////////////////////////////////////////////////////////////////////////////
