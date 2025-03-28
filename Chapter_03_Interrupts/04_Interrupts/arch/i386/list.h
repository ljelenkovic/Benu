/* Simplified from: https://github.com/torvalds/linux/blob/master/include/linux/list.h */

#ifndef _LINUX_LIST_H
#define _LINUX_LIST_H

/* Circular doubly linked list implementation. */

/* Structure for list header and list element embedded in some object */
struct list_head {
	struct list_head *next, *prev;
};

/*
 * Example:
	struct list_head _head;

	struct object {
		...
		struct list_head list_item; //or some other name
		...
   	} _obj1, _obj2, _obj3;
	... //initialize objects

	//since we usually have pointers:
	struct list_head *head = &_head;
	struct object *obj1 = &_obj1, *obj2 = &_obj2, *obj3 = &_obj3;

 * 1. use list manually (just to demonstrate connections):
	head->prev = head->next = &head; //empty list

	//add obj1:
	head->prev = &obj1->list_item;
	head->next = &obj1->list_item;
	obj1->list_item.prev = head;
	obj1->list_item.next = head;

	//add obj2 to tail:
	head->prev = &obj2->list_item;
	obj1->list_item.next = &obj2->list_item;
	obj2->list_item.prev = &obj1->list_item;
	obj2->list_item.next = head;

	//add obj3 between obj1 and obj2:
	obj1->list_item.next = &obj3->list_item;
	obj3->list_item.prev = &obj1->list_item;
	obj3->list_item.next = &obj2->list_item;
	obj2->list_item.prev = &obj3->list_item;

 * 2. use list with macros and functions (intended way):
 	LIST_HEAD(_head); //only for static initialization
	//or INIT_LIST_HEAD(&_head) for non-static initialization

	list_add(&obj1->list_item, head);		//add obj1 after 'head'
	list_add_tail(&obj2->list_item, head);		//add obj2 before 'head'
	list_add(&obj3->list_item, &obj1->list_item);	//add obj3 after 'obj1'
 */

#define LIST_HEAD(name) \
	struct list_head name = { &(name), &(name) }

static inline void INIT_LIST_HEAD(struct list_head *list)
{
	list->next = list;
	list->prev = list;
}

/* Internal function: insert 'new' between 'prev' and 'next' */
static inline void __list_add(struct list_head *new,
			      struct list_head *prev,
			      struct list_head *next)
{
	next->prev = new;
	new->next = next;
	new->prev = prev;
	prev->next = new;
}

/* list_add - add a 'new' entry after 'head' */
static inline void list_add(struct list_head *new, struct list_head *head)
{
	__list_add(new, head, head->next);
}

/* list_add_tail - add a new entry */
static inline void list_add_tail(struct list_head *new, struct list_head *head)
{
	__list_add(new, head->prev, head);
}

#define LIST_POISON1  ((void *) 0x100)
#define LIST_POISON2  ((void *) 0x122)

/* list_del - deletes entry from list */
static inline void list_del(struct list_head *entry)
{
	entry->next->prev = entry->prev;
	entry->prev->next = entry->next;
	entry->next = LIST_POISON1;
	entry->prev = LIST_POISON2;
}

/* list_is_first -- tests whether @list is the first entry in list @head */
static inline int list_is_first(const struct list_head *list, const struct list_head *head)
{
	return list->prev == head;
}

/* list_is_last - tests whether @list is the last entry in list @head */
static inline int list_is_last(const struct list_head *list, const struct list_head *head)
{
	return list->next == head;
}

/* list_is_head - tests whether @list is the list @head */
static inline int list_is_head(const struct list_head *list, const struct list_head *head)
{
	return list == head;
}

static inline int list_empty(const struct list_head *head)
{
	return head->next == head;
}


/* MACROS; macros with suffix 'entry' require/return pointer to object in list */

/* helper macro */
#define container_of(ptr, type, member) ({		\
	void *__mptr = (void *)(ptr);			\
	((type *)(__mptr - __builtin_offsetof(type, member))); })

/*
 * list_entry - get the struct for this entry
 * @ptr:	the &struct list_head pointer.
 * @type:	the type of the struct this is embedded in.
 * @member:	the name of the list_head within the struct.
 */
#define list_entry(ptr, type, member) \
	container_of(ptr, type, member)
/* e.g.:
	struct list_head *ptr = &obj1->list_item;
	//assume now that we dont have obj1, but only ptr (e.g. in some function, loop)
	struct object *obj = list_entry(ptr, struct object, list_item);
*/

/* list_first_entry - get the first element from a list header */
#define list_first_entry(ptr, type, member) \
	list_entry((ptr)->next, type, member)

/* list_last_entry - get the last element from a list */
#define list_last_entry(ptr, type, member) \
	list_entry((ptr)->prev, type, member)

/* list_first_entry_or_null - get the first element from a list */
#define list_first_entry_or_null(ptr, type, member) ({ \
	struct list_head *head__ = (ptr); \
	struct list_head *pos__ = head__->next; \
	pos__ != head__ ? list_entry(pos__, type, member) : NULL; \
})

/* list_next_entry - get the next element in list */
#define list_next_entry(pos, member) \
	list_entry((pos)->member.next, typeof(*(pos)), member)

/* list_next_entry_circular - get the next element in list */
#define list_next_entry_circular(pos, head, member) \
	(list_is_last(&(pos)->member, head) ? \
	list_first_entry(head, typeof(*(pos)), member) : list_next_entry(pos, member))

/* list_prev_entry - get the prev element in list */
#define list_prev_entry(pos, member) \
	list_entry((pos)->member.prev, typeof(*(pos)), member)

/*
 * list_for_each - iterate over a list
 * @pos:	the &struct list_head to use as a loop cursor.
 * @head:	the head for your list.
 */
#define list_for_each(pos, head) \
	for (pos = (head)->next; !list_is_head(pos, (head)); pos = pos->next)
/* e.g.:
	struct list_head *iter; //head is list header
	list_for_each(iter, head) {
		struct object *obj = list_entry(iter, struct object, list_item);
		//do something with obj
	}
*/

/* list_for_each_continue - continue iteration over a list (we don't start from head) */
#define list_for_each_continue(pos, head) \
	for (pos = pos->next; !list_is_head(pos, (head)); pos = pos->next)

/* list_for_each_prev - iterate over a list backwards */
#define list_for_each_prev(pos, head) \
	for (pos = (head)->prev; !list_is_head(pos, (head)); pos = pos->prev)

/* list_for_each_safe - iterate over a list safe against removal of list entry */
#define list_for_each_safe(pos, n, head) \
	for (pos = (head)->next, n = pos->next; \
	     !list_is_head(pos, (head)); \
	     pos = n, n = pos->next)
/* e.g.:
	struct list_head *pos, *n; //head is list header
	list_for_each_safe(pos, n, head) {
		struct object *obj = list_entry(pos, struct object, list_item);
		//do something with obj
		list_del(pos);
	}
*/

/* list_count_nodes - count nodes in the list */
static inline size_t list_count_nodes(struct list_head *head)
{
	struct list_head *pos;
	size_t count = 0;

	list_for_each(pos, head)
		count++;

	return count;
}

#define list_entry_is_head(pos, head, member)				\
	(&pos->member == (head))

/* list_for_each_entry - iterate over list of given type */
#define list_for_each_entry(pos, head, member)				\
	for (pos = list_first_entry(head, typeof(*pos), member);	\
	     !list_entry_is_head(pos, head, member);			\
	     pos = list_next_entry(pos, member))
/* e.g.:
	struct object *obj; //head is list header
	list_for_each_entry(obj, head, list_item) {
		//do something with obj
	}
*/

#endif
