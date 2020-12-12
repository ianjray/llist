#include "llist.h"

#include <stdlib.h>
#include <string.h>

#ifdef ATARI
typedef unsigned * uintptr_t;
#endif

/* Iterator has the same layout as list_node.
 * Clients only ever see a pointer-to-iterator, thus the implementation is opaque. */
struct list_iter {
    struct list_node node;
};

struct list {
    /* This implementation uses a dummy 'sentinel' node, in order to simplify link management.
     *
     * Base case:
     *           +------+
     *           v      |
     *     +--------+   |
     *     |    next|---+
     *     |sentinel|
     * +---|prev    |
     * |   +--------+
     * |      ^
     * +------+
     *
     * General case:
     *           +--------------------+
     *           v                    |
     *     +--------+    +--------+   |
     *     |    next|--->|    next|---+
     *     |sentinel|    |  node  |
     * +---|prev    |<---|prev    |
     * |   +--------+    +--------+
     * |                    ^
     * +--------------------+
     */
    struct list_node sentinel;
    size_t size;
    size_t offset;
};

struct list *list_new(size_t offset)
{
    struct list *l = calloc(1, sizeof(struct list));
    l->sentinel.prev = &l->sentinel;
    l->sentinel.next = &l->sentinel;
    l->sentinel.list = l;
    l->offset = offset;
    return l;
}

void list_delete(struct list **pl)
{
    if (pl) {
        struct list *l = *pl;
        if (l) {
            memset(l, 0, sizeof(*l));
            free(l);
            *pl = NULL;
        }
    }
}

size_t list_size(const struct list *l)
{
    if (l) {
        return l->size;
    }
    return 0;
}

struct list_iter *list_begin(struct list *l)
{
    if (l) {
        return (struct list_iter *)l->sentinel.next;
    }
    return NULL;
}

struct list_iter *list_end(struct list *l)
{
    if (l) {
        return (struct list_iter *)&l->sentinel;
    }
    return NULL;
}

static struct list_node *impl_link_of(const struct list *l, void *element)
{
    return (struct list_node *)((uintptr_t *)element + l->offset / sizeof(uintptr_t));
}

static void impl_insert_element_before(struct list_iter *iter, void *element)
{
    struct list *l = iter->node.list;
    struct list_node *link = impl_link_of(l, element);
    struct list_node *rhs = &iter->node;
    struct list_node *lhs = rhs->prev;

    /*
     *     +-------+    +--------+    +-------+
     * --->|       |-4->|    next|-1->|       |--->
     *     |  lhs  |    |  link  |    |  rhs  |
     * <---|       |<-2-|prev    |<-3-|       |<---
     *     +-------+    +--------+    +-------+
     */
    link->next = rhs;  // 1
    link->prev = lhs;  // 2
    link->list = l;
    rhs->prev = link;  // 3
    lhs->next = link;  // 4
    l->size++;
}

void *list_insert(struct list_iter *iter, void *element)
{
    if (iter && iter->node.list && element) {
        impl_insert_element_before(iter, element);
    }
    return element;
}

static void *impl_unlink(struct list_iter *iter)
{
    if (iter && iter->node.list && iter != list_end(iter->node.list)) {
        iter->node.prev->next = iter->node.next;
        iter->node.next->prev = iter->node.prev;
        iter->node.list->size--;
        return list_at(iter);
    }
    return NULL;
}

void *list_unlink(struct list_iter *iter)
{
    return impl_unlink(iter);
}

void list_erase_range(struct list_iter *it, struct list_iter *end, void (*destructor)(void *))
{
    if (it && end && destructor) {
        while (it != end) {
            it = list_next(it);
            destructor(list_unlink(list_prev(it)));
        }
    }
}

struct list_iter *list_iter_move(struct list_iter *iter, ssize_t offset)
{
    if (iter) {
        struct list *l = iter->node.list;
        if (l) {
            while (offset < 0 && iter != list_begin(l)) {
                iter = (struct list_iter *)iter->node.prev;
                offset++;
            }

            while (offset > 0 && iter != list_end(l)) {
                iter = (struct list_iter *)iter->node.next;
                offset--;
            }
        }
    }

    return iter;
}

void *list_at(struct list_iter *iter)
{
    if (iter && iter->node.list && iter != list_end(iter->node.list)) {
        return (uintptr_t *)iter - iter->node.list->offset / sizeof(uintptr_t);
    }
    return NULL;
}

void list_splice(struct list_iter *iter, struct list_iter *source_iter)
{
    if (iter && source_iter) {
        if (iter != source_iter) {
            if (((struct list_node *)iter)->list == ((struct list_node *)source_iter)->list) {
                impl_insert_element_before(iter, list_unlink(source_iter));
            }
        }
    }
}
