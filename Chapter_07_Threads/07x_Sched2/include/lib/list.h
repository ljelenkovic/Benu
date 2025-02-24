/*!
 * Simple list implementation
 *
 * properties:
 * - double linked list
 * - unsorted (FIFO & LIFO) or sorted list (compare function must be provided)
 * - list header: list_t type
 * - elements (objects/structures) for list must have list_t element included
 *  (spare extra call to malloc and free; save memory)
 *
 * List usage on an example

 List element must be included in object that we want to put in list. Place and
 variable name is unimportant, but must be used when calling list functions.

 Simple list width only two elements will look like:

 struct some_object {
	 int something;
	 ...
	 list_h le1; -- list element 1 for list1
	 ...
 } object1, object2;

 list_t some_list1;

 --when list is formed and both objects are in list, data will look like:

 some_list1.first = &object1.le1
 some_list1.last  = &object2.le1

 object1.le1.prev = NULL;			object2.le1.prev = &object2.le1;
 object1.le1.next = &object2.le1;		object2.le1.next = NULL;
 object1.le1.object = &object1			object2.le1.object = &object2;

 Same object can be in multiple list simultaneously if it have multiple list_h
 element data member (e.g. le2, le3).
*/
#pragma once

#include <types/basic.h>

/*! List element pointers */
typedef struct _list_h_
{
	struct _list_h_  *prev;
			  /* pointer to previous list element */

	struct _list_h_  *next;
			  /* pointer to next list element */

	void 		 *object;
			  /* pointer to object (which contains this list_h) */
}
list_h;

/*! list header type */
typedef struct _list_
{
	list_h  *first;
	list_h  *last;
}
list_t;

/* for static list elements initialization */
#define LIST_H_NULL	{NULL, NULL, NULL}

/* for static list initialization (empty list) */
#define LIST_T_NULL	{NULL, NULL}

#define FIRST	0	/* get first or last list element */
#define LAST	1

void list_init(list_t *list);

/*! Add element to list, add to tail - as last element */
void list_append(list_t *list, void *object, list_h *hdr);

/*! Add element to list, add to head - as first element */
void list_prepend(list_t *list, void *object, list_h *hdr);

/*! Add element to sorted list */
void list_sort_add(list_t *list, void *object, list_h *hdr,
				   int(*cmp)(void *, void *));

/*! Get pointer to first or last list element */
void *list_get(list_t *list, unsigned int flags);

/*! Get pointer to next object in list */
void *list_get_next(list_h *hdr);

/*!
 * Remove element from list: FIRST, LAST or given (ref)
 * NOTE function assumes that ref element is in list - it doesn't check!!!
 */
void *list_remove(list_t *list, unsigned int flags, list_h *ref);

/*! Find element in list (returns pointer to object, NULL if not found) */
void *list_find(list_t *list, list_h *ref);

/*! Remove element from list if element is in list */
void *list_find_and_remove(list_t *list, list_h *ref);
