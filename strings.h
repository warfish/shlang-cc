/*
 * strings.h
 * Stored string interface
 */

#pragma once

typedef struct string
{
    const char* ptr; // Unique pointer value
} string_t;

// S is a pretty common letter, so lets make sure we didn't override anything
#ifdef _S
#   error _S is already defined 
#endif 

#ifdef _MAKESTR
#   error _MAKESTR is already defined
#endif 

#define _S(str) ((str).ptr)
#define _MAKESTR(str) (string_t){(str)}

/**
 * \brief   Initialize string storage
 */
int strings_init(void);

/**
 * \brief   Store a new string.
 *
 * \return  If any stored string compares equal to 'str', function will return already stored copy.
 *          Otherwise a new string is created and returned.
 */
string_t string(const char* str);

/**
 * \brief   A dictionary maps string_t values to opaque data values
 */
typedef struct dict dict_t;

/**
 * \brief   Create new dictionary
 */
dict_t* dict_create(void);

/**
 * \brief   Insert a new dictionary value
 * \return  0 on success, system error code on failure (ENOMEM)
 */
int dict_insert(dict_t* dict, string_t str, void* val);

/**
 * \brief   Look for existing dictionary value
 */
void* dict_search(dict_t* dict, string_t str);

/**
 * \brief   Remove value from a dictionary
 */
void dict_remove(dict_t* dict, string_t key);

/**
 * \brief   Release all dictionary resources
 */
void dict_destroy(dict_t* dict);
