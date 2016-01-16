/*
 * Single and double intrusive linked list implementation
 */

#pragma once

#include <stddef.h>

typedef struct slist_head
{
	struct slist_head* next;
} slist_head;

#if !defined(containerof)
#	define containerof(ptr, type, member) \
        ((type *) ((char *)(ptr) - offsetof(type, member))) //(type *)((char *)(1 ? (ptr) : &((type *)0)->member) - offsetof(type, member))
#endif 

#define slist_entry(ptr_, type_, name_) containerof(ptr_, type_, name_)

static inline void slist_init(slist_head* head)
{
	head->next = NULL;
}

static inline void slist_insert(slist_head* head, slist_head* p)
{
	p->next = head->next;
	head->next = p;
}
