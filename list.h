/*
 * Single and double intrusive linked list implementation
 */

#pragma once

#include <stddef.h>
#include <stdbool.h>
 
#include "support.h"

typedef struct slist_head
{
	struct slist_head* next;
} slist_head;

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

static inline void slist_remove(slist_head* head, slist_head* p)
{
	head->next = p->next;
}

typedef struct list_head
{
	struct list_head* next;
	struct list_head* prev;
} list_head;

#define list_entry(ptr_, type_, name_) containerof(ptr_, type_, name_)

static inline void list_init(list_head* head)
{
	head->next = NULL;
	head->prev = NULL;
}

static inline void list_insert(list_head* head, list_head* p)
{
	list_head* next = head->next;
	
	head->next = p;
	p->prev = head;

	p->next = next;
	if (next) {
		next->prev = p;
	}
}

static inline void list_remove(list_head* p)
{
	list_head* next = p->next;
	list_head* prev = p->prev;

	if (next) {
		next->prev = prev;
	}

	if (prev) {
		prev->next = next;
	}
}

static inline bool list_empty(list_head* head)
{
	return (head->next == NULL) && (head->prev == NULL);
}
