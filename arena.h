/*
 * arena.h
 * Simple arena allocator interface.
 */

#pragma once

#include <stdlib.h>
 
/**
 * \brief	Opaque arena handle
 */
typedef struct arena arena_t;

/**
 * \brief 	Create new arena with default maximum block size
 */
arena_t* arena_create(void);

/**
 * \brief	Free existing arena and all allocated blocks
 */
void arena_destroy(arena_t* arena);

/**
 * \brief 	Allocate a block of memory inside an arena
 */
void* arena_alloc(arena_t* arena, size_t bytes);

