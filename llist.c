#include "llist.h"

#include <stdlib.h>


struct list_node {
    struct list_node *prev;
    struct list_node *next;
    /* Iterator @c list_iter is defined as a pointer to a node, in order to provide a flexible API that does not require extra allocations.
     * The list pointer is needed to find the parent list (and, notably, the sentinel object) in the iterator API. */
    struct list *list;
    void *userdata;
};

/* Iterator has the same layout as node.
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
    size_t element_size;
    void (*element_destructor)(void *);
};

static struct {
    void *(*alloc)(size_t);
    void (*dealloc)(void *);
} mem = {
    malloc,
    free
};


void list_allocator_set(
        void *(*alloc)(size_t),
        void (dealloc)(void *))
{
    mem.alloc = alloc;
    mem.dealloc = dealloc;
}

struct list *list_new(
        size_t element_size,
        void (element_destructor)(void *))
{
    struct list *l = mem.alloc(sizeof(struct list));
    l->sentinel.prev = &l->sentinel;
    l->sentinel.next = &l->sentinel;
    l->sentinel.list = l;
    l->size = 0;
    l->element_size = offsetof(struct list_node, userdata) + element_size;
    l->element_destructor = element_destructor;
    return l;
}

void list_delete(struct list *l)
{
    list_clear(l);
    mem.dealloc(l);
}

void *list_front(struct list *l)
{
    if (l && l->size > 0) {
        return &l->sentinel.next->userdata;
    }
    return NULL;
}

void *list_back(struct list *l)
{
    if (l && l->size > 1) {
        return &l->sentinel.prev->userdata;
    }
    return NULL;
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

void *list_iter_deref(struct list_iter *iter)
{
    if (iter && iter != list_end(iter->node.list)) {
        return &iter->node.userdata;
    }
    return NULL;
}

struct list_iter *list_iter_inc(struct list_iter *iter)
{
    return list_iter_advance(iter, 1);
}

struct list_iter *list_iter_dec(struct list_iter *iter)
{
    return list_iter_advance(iter, -1);
}

struct list_iter *list_iter_advance(struct list_iter *iter, ssize_t distance)
{
    if (iter) {
        struct list *l = iter->node.list;
        if (l) {
            while (distance < 0 && iter != list_begin(l)) {
                iter = (struct list_iter *)iter->node.prev;
                distance++;
            }

            while (distance > 0 && iter != list_end(l)) {
                iter = (struct list_iter *)iter->node.next;
                distance--;
            }
        }
    }

    return iter;
}

bool list_empty(const struct list *l)
{
    if (l) {
        return l->size == 0;
    }
    return true;
}

size_t list_size(const struct list *l)
{
    if (l) {
        return l->size;
    }
    return 0;
}

static void impl_release(struct list *l, struct list_node *node)
{
    if (l->element_destructor) {
        l->element_destructor(&node->userdata);
    }
    mem.dealloc(node);
}

static void impl_unlink(struct list *l, struct list_node *node)
{
    if (l && node && l == node->list) {
        node->prev->next = node->next;
        node->next->prev = node->prev;
        l->size--;
        impl_release(l, node);
    }
}

void list_clear(struct list *l)
{
    while (!list_empty(l)) {
        list_pop_front(l);
    }
}

static void *impl_insert_node_before(struct list *l, struct list_iter *iter, struct list_node *node)
{
    struct list_node *rhs = (struct list_node *)iter;
    struct list_node *lhs = rhs->prev;

    /*
     *     +-------+    +--------+    +-------+
     * --->|       |-4->|    next|-1->|       |--->
     *     |  lhs  |    |  node  |    |  rhs  |
     * <---|       |<-2-|prev    |<-3-|       |<---
     *     +-------+    +--------+    +-------+
     */
    node->next = rhs;  // 1
    node->prev = lhs;  // 2
    rhs->prev = node;  // 3
    lhs->next = node;  // 4

    node->list = l;
    l->size++;
    return &node->userdata;
}

static void *impl_insert_before(struct list *l, struct list_iter *iter)
{
    if (l && iter) {
        struct list_node *node = mem.alloc(l->element_size);
        return impl_insert_node_before(l, iter, node);
    } else {
        return NULL;
    }
}

void *list_insert(struct list *l, struct list_iter *iter)
{
    return impl_insert_before(l, iter);
}

void list_erase(struct list *l, struct list_iter *iter)
{
    if (l && iter && iter != list_end(l)) {
        impl_unlink(l, (struct list_node *)iter);
    }
}

void *list_push_back(struct list *l)
{
    return impl_insert_before(l, list_end(l));
}

void list_pop_back(struct list *l)
{
    if (l && l->sentinel.prev != &l->sentinel) {
        impl_unlink(l, l->sentinel.prev);
    }
}

void *list_push_front(struct list *l)
{
    return impl_insert_before(l, list_begin(l));
}

void list_pop_front(struct list *l)
{
    if (l && l->sentinel.next != &l->sentinel) {
        impl_unlink(l, l->sentinel.next);
    }
}

void list_splice(struct list *l, struct list_iter *iter, struct list_iter *source_iter)
{
    if (l && iter && source_iter) {
        if (l == ((struct list_node *)iter)->list && l == ((struct list_node *)source_iter)->list) {
            if (iter != source_iter) {
                struct list_node *source_node = (struct list_node *)source_iter;
                source_node->prev->next = source_node->next;
                source_node->next->prev = source_node->prev;
                l->size--;
                impl_insert_node_before(l, iter, source_node);
            }
        }
    }
}
