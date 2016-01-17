/*
 * "Generic" small object set
 * User defines a set of macros and includes this file to produce a specification:
 * - HASH_KEY_TYPE type of stored key values
 * - HASH_VALUE_TYPE type of stored values
 * - HASH_KEY_CMP_FUNC name of the key comparison function: bool(HASH_KEY_CMP_FUNC)(HASH_KEY_TYPE lhv, HASH_KEY_TYPE rhv)
 * - HASH_FUNC name of the key hash function: uint32_t(HASH_FUNC)(HASH_KEY_TYPE key)
 * - HASH_PREFIX name prefix to attach to generated hash types and function: HASH_PREFIX_insert, HASH_PREFIX_search, etc
 */

#include "limits.h"

#if !defined(HASH_KEY_TYPE)
#   error HASH_KEY_TYPE should be defined
#endif 

#if !defined(HASH_VALUE_TYPE)
#   error HASH_VALUE_TYPE should be defined
#endif 

#if !defined(HASH_KEY_CMP_FUNC)
#   error HASH_FUNC should be defined
#endif 

#if !defined(HASH_FUNC)
#   error HASH_FUNC should be defined
#endif 

#if !defined(HASH_PREFIX)
#   error HASH_PREFIX must be defined
#endif 

#if !defined(HASH_MAKE_PREFIX)
#   define _CONCAT(pfx, body)               pfx##body 
#   define HASH_MAKE_PREFIX1(pfx, body)     _CONCAT(pfx, body)
#   define HASH_MAKE_PREFIX(body)           HASH_MAKE_PREFIX1(HASH_PREFIX, body)
#endif

#if !defined(HASH_BUCKETS)
#   define HASH_BUCKETS 256
#   define HASH_BUCKET(key) (HASH_FUNC((key)) & (HASH_BUCKETS - 1))
#endif

#if !defined(HASH_CHUNK_LENGTH)
#   define HASH_CHUNK_LENGTH 16 // Keys per chunk
#endif

/**
 * Generated hash table structure 
 */
typedef struct HASH_PREFIX
{
    arena_t* arena;                     // Arena to allocate entries
    slist_head buckets[HASH_BUCKETS]; 
} HASH_MAKE_PREFIX(_t);

/**
 * Generated hash entry
 * To make things more cache-friendly we allocate a block of multiple stored entries and link those in a chain
 */
typedef struct 
{
    slist_head link;                        // Chain link. 
                                            // Keeping it at the top of the structure, shaves off some cache-misses later when traversing the list
    uint16_t bitmap;                        // bit[N] indicates if key/value slots N are occupied(1) or free(0). 
                                            // Total slots are defined by HASH_CHUNK_LENGTH
    HASH_KEY_TYPE keys[HASH_CHUNK_LENGTH];  
    HASH_VALUE_TYPE values[HASH_CHUNK_LENGTH]; 
} HASH_MAKE_PREFIX(_entry_t);

_Static_assert((sizeof(((HASH_MAKE_PREFIX(_entry_t)*)0)->bitmap) * CHAR_BIT) == HASH_CHUNK_LENGTH, "Invalid bitmap storage size");

HASH_MAKE_PREFIX(_t)* HASH_MAKE_PREFIX(_create)(void)
{
    HASH_MAKE_PREFIX(_t)* h = (HASH_MAKE_PREFIX(_t)*) calloc(1, sizeof(*h));
    if (!h) {
        return NULL;
    }

    h->arena = arena_create();
    if (!h->arena) {
        free(h);
        return NULL;
    }

    return h;
}

static HASH_MAKE_PREFIX(_entry_t)* HASH_MAKE_PREFIX(_create_entry)(HASH_MAKE_PREFIX(_t)* hash)
{
    // TODO: 64-byte aligned alloc
    HASH_MAKE_PREFIX(_entry_t)* entry = arena_alloc(hash->arena, sizeof(*entry));
    if (!entry) {
        return NULL;
    }

    entry->bitmap = 0;
    return entry;
}

// Search key in this entry
// Returns a slot number or -1 if key was not found
static int HASH_MAKE_PREFIX(_scan_entry)(HASH_MAKE_PREFIX(_entry_t)* entry, HASH_KEY_TYPE key)
{
    if (entry->bitmap == 0) {
        return -1;
    }

    uint16_t bitmap = entry->bitmap;
    while (bitmap) {
        int i = __builtin_ffs(bitmap) - 1;
        if (HASH_KEY_CMP_FUNC(entry->keys[i], key)) {
            return i;
        }

        // Clear this bit and try next one
        // Hardware ffs is O(1), so it doesn't matter if we always start at the beginning
        bitmap &= ~(1ul << i);
    }
    
    return -1;
}

// Returns -1  if there is no place in entry to store a new value or index of the slot used
static int HASH_MAKE_PREFIX(_store_value)(HASH_MAKE_PREFIX(_entry_t)* entry, HASH_KEY_TYPE key, HASH_VALUE_TYPE val)
{
    if (entry->bitmap == UINT16_MAX) {
        return -1;
    }

    int i = __builtin_ffs(~entry->bitmap) - 1; // Should not return 0, since we've checked this case above
    entry->keys[i] = key;
    entry->values[i] = val;
    entry->bitmap |= (1ul << i);
    return i;
}

int HASH_MAKE_PREFIX(_insert)(HASH_MAKE_PREFIX(_t)* hash, HASH_KEY_TYPE key, HASH_VALUE_TYPE val)
{
    if (!hash) {
        return EINVAL;
    }

    uint32_t h = HASH_BUCKET(key);

    // We are following set semantics - scan for duplicate and update its value if found
    // TODO: rethink this once we get to actual use cases
    slist_for_each(hash->buckets[h], p) {
        HASH_MAKE_PREFIX(_entry_t)* entry = list_entry(p, HASH_MAKE_PREFIX(_entry_t), link);
        int i = HASH_MAKE_PREFIX(_scan_entry)(entry, key);
        if (i >= 0) {
            entry->values[i] = val;
            return 0;
        }
    }

    // See if we have space available anywhere in existing entries
    slist_for_each(hash->buckets[h], p) {
        HASH_MAKE_PREFIX(_entry_t)* entry = list_entry(p, HASH_MAKE_PREFIX(_entry_t), link);
        if (HASH_MAKE_PREFIX(_store_value)(entry, key, val) >= 0) {
            return 0;
        }
    }

    // Create a new entry
    HASH_MAKE_PREFIX(_entry_t)* entry = HASH_MAKE_PREFIX(_create_entry)(hash);
    if (!entry) {
        return ENOMEM;
    }
    
    HASH_MAKE_PREFIX(_store_value)(entry, key, val);
    slist_insert(&hash->buckets[h], &entry->link);
    return 0;
}

HASH_VALUE_TYPE HASH_MAKE_PREFIX(_search)(HASH_MAKE_PREFIX(_t)* hash, HASH_KEY_TYPE key)
{
    if (!hash) {
        return NULL;
    }

    uint32_t h = HASH_BUCKET(key);

    slist_for_each(hash->buckets[h], p) {
        HASH_MAKE_PREFIX(_entry_t)* entry = list_entry(p, HASH_MAKE_PREFIX(_entry_t), link);
        int i = HASH_MAKE_PREFIX(_scan_entry)(entry, key);
        if (i >= 0) {
            return entry->values[i];
        }
    }

    return NULL;
}

void HASH_MAKE_PREFIX(_remove)(HASH_MAKE_PREFIX(_t)* hash, HASH_KEY_TYPE key)
{
    if (!hash) {
        return;
    }

    uint32_t h = HASH_BUCKET(key);

    slist_for_each(hash->buckets[h], p) {
        HASH_MAKE_PREFIX(_entry_t)* entry = list_entry(p, HASH_MAKE_PREFIX(_entry_t), link);
        int i = HASH_MAKE_PREFIX(_scan_entry)(entry, key);
        if (i >= 0) {
            entry->bitmap &= ~(1ul << i); // Mark entry as empty, do not free anything yet
            break;
        }
    }
}

void HASH_MAKE_PREFIX(_destroy)(HASH_MAKE_PREFIX(_t)* hash)
{
    if (hash) {
        arena_destroy(hash->arena);
        free(hash);
    }
}
