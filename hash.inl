/*
 * "Generic" hash table
 * User defined a set of macros and includes this file to produce a specification
 *
 * TODO: Implementation is very simplistic, work on cache awareness
 */

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
#endif

typedef struct HASH_PREFIX
{
    arena_t* arena;
    slist_head buckets[HASH_BUCKETS];
} HASH_MAKE_PREFIX(_t);

typedef struct 
{
    HASH_KEY_TYPE key;
    HASH_VALUE_TYPE val;
    slist_head link;
} HASH_MAKE_PREFIX(_entry_t);

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

int HASH_MAKE_PREFIX(_insert)(HASH_MAKE_PREFIX(_t)* hash, HASH_KEY_TYPE key, HASH_VALUE_TYPE val)
{
    if (!hash) {
        return EINVAL;
    }

    uint32_t h = HASH_FUNC(key) & (HASH_BUCKETS - 1);

    // Duplicate?
    slist_for_each(hash->buckets[h], p) {
        HASH_MAKE_PREFIX(_entry_t)* entry = list_entry(p, HASH_MAKE_PREFIX(_entry_t), link);
        if (HASH_KEY_CMP_FUNC(entry->key, key)) {
            entry->val = val;
            return 0;
        }
    }

    HASH_MAKE_PREFIX(_entry_t)* entry = arena_alloc(hash->arena, sizeof(*entry));
    if (!entry) {
        return ENOMEM;
    }

    entry->key = key;
    entry->val = val;
    slist_insert(&hash->buckets[h], &entry->link);

    return 0;
}

HASH_VALUE_TYPE HASH_MAKE_PREFIX(_search)(HASH_MAKE_PREFIX(_t)* hash, HASH_KEY_TYPE key)
{
    if (!hash) {
        return NULL;
    }

    uint32_t h = HASH_FUNC(key) & (HASH_BUCKETS - 1);

    slist_for_each(hash->buckets[h], p) {
        HASH_MAKE_PREFIX(_entry_t)* entry = list_entry(p, HASH_MAKE_PREFIX(_entry_t), link);
        if (HASH_KEY_CMP_FUNC(entry->key, key)) {
            return entry->val;
        }
    }

    return NULL;
}

void HASH_MAKE_PREFIX(_remove)(HASH_MAKE_PREFIX(_t)* hash, HASH_KEY_TYPE key)
{
    if (!hash) {
        return;
    }

    uint32_t h = HASH_FUNC(key) & (HASH_BUCKETS - 1);

    slist_head* p = hash->buckets[h].next;
    slist_head* prev = &hash->buckets[h];
    while (p != NULL) {
        HASH_MAKE_PREFIX(_entry_t)* entry = list_entry(p, HASH_MAKE_PREFIX(_entry_t), link);
        if (HASH_KEY_CMP_FUNC(entry->key, key)) {
            slist_remove(prev, p);
            arena_free(hash->arena, entry);
            break;
        }

        prev = p;
        p = p->next;
    }
}

void HASH_MAKE_PREFIX(_destroy)(HASH_MAKE_PREFIX(_t)* hash)
{
    if (hash) {
        arena_destroy(hash->arena);
        free(hash);
    }
}
