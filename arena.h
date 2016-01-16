/*
 * arena.h
 * Simple arena allocator interface.
 */

#pragma once

#include <stdlib.h>
 
/**
 * \brief   Opaque arena handle
 */
typedef struct arena arena_t;

/**
 * \brief   Create new arena with default maximum block size
 */
arena_t* arena_create(void);

/**
 * \brief   Allocate a block of memory inside an arena
 */
void* arena_alloc(arena_t* arena, size_t bytes);

/**
 * \brief   Free allocated block
 *
 * To speed up possible future allocations freed blocks may not immediately be returned to system allocator
 * To release unused memory call @arena_trim
 */
void arena_free(arena_t* arena, void* ptr);

/**
 * \brief   Return freed blocks to system allocator
 */
void arena_trim(arena_t* arena);

/**
 * \brief   Free existing arena and all allocated blocks
 */
void arena_destroy(arena_t* arena);
