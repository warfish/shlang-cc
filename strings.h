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
 * \return  If @str@ already exists, this function returns a unique pointer to previously allocated string.
 *          Otherwise a new 
 */
string_t string(const char* str);

/**
 * \brief   A dictionary maps string_t values to opaque data values
 */
typedef struct dict dict_t;

dict_t* dict_create(void);

int dict_insert(dict_t* dict, string_t str, void* val);

void* dict_search(dict_t* dict, string_t str);

void dict_remove(dict_t* dict, string_t key);

void dict_destroy(dict_t* dict);

