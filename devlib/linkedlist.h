/*
 * #SOFTWARE_NAME#
 * (copied from debuglib)
 *
 * Linked list macros.
 *
 * In order to use the following macros, you need to define next and prev
 * in struct:
 * struct x { struct x *next; struct x *prev; ... }
 *
 * License: MIT, see COPYING
 * Authors: Antti Partanen <aehparta@iki.fi, duge at IRCnet>
 */

#ifndef __LINKEDLIST_H__
#define __LINKEDLIST_H__

struct ll_str
{
	char *str;
	struct ll_str *prev;
	struct ll_str *next;
};

/**
 * Append new item into list.
 */
#define LL_APP(first, last, item) \
do { \
    item->prev = NULL; \
    item->next = NULL; \
    if (first == NULL) first = item; \
    else { \
        last->next = item; \
        item->prev = last; \
    } \
    last = item; \
} while(0)
#define LL_APP_SAFE(mutex, first, last, item) \
do { \
    mutex_lock(&mutex); \
    LL_APP(first, last, item); \
    mutex_unlock(&mutex); \
} while (0)

/**
 * Insert item into queue using field in struct to determine correct position.
 * field should be integer or similar and the item is inserted after first field
 * that has same or smaller value in list that the item being inserted.
 */
#define LL_INS(first, last, item, loop) \
do { \
    item->prev = NULL; \
    item->next = NULL; \
    loop = last; \
    while (loop) \
    { \
        if (loop->position <= item->position) \
        { \
            if (loop->next) \
            { \
                loop->next->prev = item; \
            } \
            item->next = loop->next; \
            item->prev = loop; \
            loop->next = item; \
            if (last == loop) last = item; \
            break; \
        } \
        loop = loop->prev; \
        if (loop == NULL) \
        { \
            item->next = first; \
            first->prev = item; \
            first = item; \
            break; \
        } \
    } \
    if (last == NULL) \
    { \
        first = last = item; \
    } \
} while(0)


/**
 * Get item from top of the queue.
 */
#define LL_GET(first, last, item) \
do { \
    item = NULL; \
    if (first != NULL) \
    { \
        item = first; \
        if (first->next == NULL) \
        { \
            first = NULL; \
            last = NULL; \
        } \
        else { \
            first = first->next; \
            first->prev = NULL; \
        } \
    } \
} while (0)
#define LL_GET_SAFE(mutex, first, last, item) \
do { \
    mutex_lock(&mutex); \
    LL_GET(first, last, item); \
    mutex_unlock(&mutex); \
} while (0)


/**
 * Count items in list.
 */
#define LL_COUNT(first, last, loop, count) \
do { \
    count = 0; \
    loop = first; \
    while (loop) { \
        count++; \
        loop = loop->next; \
    } \
} while (0)
#define LL_COUNT_SAFE(mutex, first, last, loop, count) \
do { \
    mutex_lock(&mutex); \
    LL_GET(first, last, loop, count); \
    mutex_unlock(&mutex); \
} while (0)


#endif
