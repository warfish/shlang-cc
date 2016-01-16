#include "strings.h"
#include "arena.h"
#include "list.h"
#include "test.h"

#include <stddef.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <errno.h>

//////////////////////////////////////////////////////////////////////////////

#define HASH_KEY_TYPE const char*
#define HASH_VALUE_TYPE const char*
#define HASH_FUNC strhash
#define HASH_KEY_CMP_FUNC strcomp
#define HASH_PREFIX string_table

static uint32_t strhash(const char* key) {
    return key[0] + key[strlen(key)];
}

static bool strcomp(const char* lhv, const char* rhv) {
    return 0 == strcmp(lhv, rhv);
}

#include "hash.inl"

static arena_t* g_string_arena = NULL;
static string_table_t* g_string_table = NULL;

int strings_init(void)
{
    if (g_string_table == NULL) {
        assert(g_string_arena == NULL);
        
        g_string_arena = arena_create();
        if (!g_string_arena) {
            return ENOMEM;
        }

        g_string_table = string_table_create();
        if (!g_string_table) {
            arena_destroy(g_string_arena);
            return ENOMEM;
        }
    }

    return 0;
}

string_t string(const char* str)
{
    if (!str) {
        return _MAKESTR(NULL);
    }

    const char* res = string_table_search(g_string_table, str);
    if (!res) {
        res = arena_alloc(g_string_arena, strlen(str) + 1);
        if (!res) {
            return _MAKESTR(NULL);
        }

        strcpy((char*)res, str);
        if (0 != string_table_insert(g_string_table, res, res)) {
            arena_free(g_string_arena, (void*)res);
            return _MAKESTR(NULL);
        }
    }

    return _MAKESTR(res);
}

static void strings_destroy(void)
{
    if (g_string_table) {
        assert(g_string_arena);
        arena_destroy(g_string_arena);
        string_table_destroy(g_string_table);
    }

    g_string_table = NULL;
    g_string_arena = NULL;
}

#if defined(TEST)
static void strings_test(void)
{
    int error = strings_init();
    CU_ASSERT_FALSE(error);

    string_t s1 = string("lol");
    CU_ASSERT(_S(s1) != NULL);

    string_t s2 = string("wtf");
    CU_ASSERT(_S(s2) != NULL);    
    CU_ASSERT(_S(s1) != _S(s2));

    string_t s3 = string("lol");
    CU_ASSERT(_S(s3) != NULL);
    CU_ASSERT(_S(s3) == _S(s1));

    strings_destroy();
}
TEST_ADD(strings_test);
#endif // TEST

//////////////////////////////////////////////////////////////////////////////

#undef  HASH_KEY_TYPE
#define HASH_KEY_TYPE string_t

#undef  HASH_VALUE_TYPE
#define HASH_VALUE_TYPE void*

#undef  HASH_FUNC
#define HASH_FUNC string_hash

#undef  HASH_KEY_CMP_FUNC
#define HASH_KEY_CMP_FUNC string_cmp

#undef  HASH_PREFIX
#define HASH_PREFIX dict

static inline uint32_t string_hash(string_t key) {
    return (uint32_t)(uintptr_t)(_S(key));
}

static inline bool string_cmp(string_t lhv, string_t rhv) {
    return _S(lhv) == _S(rhv);
}

#include "hash.inl"

#if defined(TEST)
static void dict_test(void)
{
    int error = strings_init();
    CU_ASSERT_FALSE(error);

    dict_t* dict = dict_create();
    CU_ASSERT(dict != NULL);

    void* val = dict_search(dict, _MAKESTR("lol"));
    CU_ASSERT_EQUAL(val, NULL);

    string_t key1 = string("lol");
    error = dict_insert(dict, key1, &key1);
    CU_ASSERT_FALSE(error);

    string_t key2 = string("wtf");
    error = dict_insert(dict, key2, &key2);
    CU_ASSERT_FALSE(error); 

    CU_ASSERT_EQUAL(dict_search(dict, key1), &key1);
    CU_ASSERT_EQUAL(dict_search(dict, key2), &key2);

    error = dict_insert(dict, key1, &key2);
    CU_ASSERT_FALSE(error);     
    CU_ASSERT_EQUAL(dict_search(dict, key1), &key2);
    CU_ASSERT_EQUAL(dict_search(dict, key2), &key2);

    dict_remove(dict, key1);
    CU_ASSERT_EQUAL(dict_search(dict, key1), NULL);
    CU_ASSERT_EQUAL(dict_search(dict, key2), &key2);

    dict_destroy(dict);
    strings_destroy();
}
TEST_ADD(dict_test);
#endif // TEST

//////////////////////////////////////////////////////////////////////////////
