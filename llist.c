#include "llist.h"

#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#ifndef SIZE_MAX
// Support compilation on Atari Lattice C.
#define SIZE_MAX ((size_t)-1)
#endif

struct list {
    /// Uses a dummy 'sentinel' node, in order to simplify link management.
    ///
    /// Base case:
    ///           +------+
    ///           v      |
    ///     +--------+   |
    ///     |    next|---+
    ///     |sentinel|
    /// +---|prev    |
    /// |   +--------+
    /// |      ^
    /// +------+
    ///
    /// General case:
    ///           +--------------------+
    ///           v                    |
    ///     +--------+    +--------+   |
    ///     |    next|--->|    next|---+
    ///     |sentinel|    |  node  |
    /// +---|prev    |<---|prev    |
    /// |   +--------+    +--------+
    /// |                    ^
    /// +--------------------+
    struct list_node sentinel;
    size_t size;
    size_t offset;
};

/// Sanity check LIST_NODE @c offset.
/// @return True if offset is valid.
static bool check_offset(size_t offset)
{
    // Ensure embedded list_node pointers are properly aligned.
    if ((offset % sizeof(void *)) != 0) {
        return false;
    }

    return true;
}

struct list *list_new(size_t offset)
{
    struct list *l;

    if (!check_offset(offset)) {
        errno = EINVAL;
        return NULL;
    }

    l = calloc(1, sizeof(struct list));
    if (!l) {
        errno = ENOMEM;
        return NULL;
    }

    l->sentinel.prev = &l->sentinel;
    l->sentinel.next = &l->sentinel;
    l->sentinel.list = l;
    l->offset = offset;
    return l;
}

void list_delete(struct list *l, void (*destructor)(void *))
{
    if (!l) {
        return;
    }

    list_clear(l, destructor);
    free(l);
}

bool list_empty(const struct list *l)
{
    return list_size(l) == 0;
}

size_t list_size(const struct list *l)
{
    if (!l) {
        return 0;
    }

    return l->size;
}

int list_clear(struct list *l, void (*destructor)(void *))
{
    if (!l) {
        errno = EINVAL;
        return -1;
    }

    while (!list_empty(l)) {
        if (list_erase(list_begin(l), destructor) < 0) {
            return -1; // UNREACHABLE
        }
    }

    return 0;
}

/// Iterator has the same layout as list_node.
/// Clients only ever see a pointer-to-iterator, thus the implementation is opaque.
struct list_iter {
    struct list_node node;
};

struct list_iter *list_begin(struct list *l)
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"

    // Const cast away is safe because @c l is non-const.
    return (struct list_iter *)list_cbegin(l);

#pragma GCC diagnostic pop
}

const struct list_iter *list_cbegin(const struct list *l)
{
    if (!l) {
        errno = EINVAL;
        return NULL;
    }

    return (const struct list_iter *)l->sentinel.next;
}

struct list_iter *list_end(struct list *l)
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"

    // Const cast away is safe because @c l is non-const.
    return (struct list_iter *)list_cend(l);

#pragma GCC diagnostic pop
}

const struct list_iter *list_cend(const struct list *l)
{
    if (!l) {
        errno = EINVAL;
        return NULL;
    }

    return (const struct list_iter *)&l->sentinel;
}

struct list_iter *list_next(struct list_iter *it)
{
    return list_advance(it, 1);
}

const struct list_iter *list_cnext(const struct list_iter *it)
{
    return list_advance_const(it, 1);
}

struct list_iter *list_prev(struct list_iter *it)
{
    return list_advance(it, -1);
}

const struct list_iter *list_cprev(const struct list_iter *it)
{
    return list_advance_const(it, -1);
}

struct list_iter *list_advance(struct list_iter *it, ssize_t n)
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"

    // Const cast away is safe because @c it is non-const.
    return (struct list_iter *)list_advance_const(it, n);

#pragma GCC diagnostic pop
}

const struct list_iter *list_advance_const(const struct list_iter *it, ssize_t n)
{
    const struct list_iter *current = it;
    struct list *l;

    if (!it) {
        errno = EINVAL;
        return NULL;
    }

    l = it->node.list;
    if (!l) {
        // Iterator not linked.
        errno = EINVAL;
        return NULL;
    }

    while (n > 0) {
        if (current == list_cend(l)) {
            errno = ERANGE;
            return NULL;
        }
        current = (const struct list_iter *)current->node.next;
        n--;
    }

    while (n < 0) {
        if (current == list_cbegin(l)) {
            errno = ERANGE;
            return NULL;
        }
        current = (const struct list_iter *)current->node.prev;
        n++;
    }

    return current;
}

struct list_iter *list_element(void *element, size_t offset)
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"

    // Const cast away is safe because @c element is non-const.
    return (struct list_iter *)list_element_const(element, offset);

#pragma GCC diagnostic pop
}

const struct list_iter *list_element_const(const void *element, size_t offset)
{
    const struct list_node *node;

    if (!element) {
        errno = EINVAL;
        return NULL;
    }

    if (!check_offset(offset)) {
        errno = EINVAL;
        return NULL;
    }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-align"

    // Pointer to LIST_NODE.
    node = (const struct list_node *)((const char *)element + offset);

#pragma GCC diagnostic pop

    // Cast to iterator.
    return (const struct list_iter *)node;
}

struct list_iter *list_insert(struct list_iter *it, void *element)
{
    struct list *l;
    struct list_node *link;
    struct list_node *rhs;
    struct list_node *lhs;

    if (!it || !element) {
        errno = EINVAL;
        return NULL;
    }

    l = it->node.list;
    if (!l) {
        // Iterator not linked.
        errno = EINVAL;
        return NULL;
    }

    if (l->size == SIZE_MAX) {
        // Detect pathological overflow case.
        errno = EOVERFLOW;
        return NULL;
    }

    ///     +-------+    +--------+    +-------+
    /// --->|       |-4->|    next|-1->|       |--->
    ///     |  lhs  |    |  link  |    |  rhs  |
    /// <---|       |<-2-|prev    |<-3-|       |<---
    ///     +-------+    +--------+    +-------+

    link = (struct list_node *)list_element(element, l->offset);
    if (!link) {
        errno = EINVAL;
        return NULL;
    }

    rhs = &it->node;
    lhs = rhs->prev;

    link->next = rhs;  // 1
    link->prev = lhs;  // 2
    link->list = l;

    rhs->prev = link;  // 3
    lhs->next = link;  // 4

    l->size++;

    return (struct list_iter *)link;
}

struct list_iter *list_push_front(struct list *l, void *element)
{
    return list_insert(list_begin(l), element);
}

struct list_iter *list_push_back(struct list *l, void *element)
{
    return list_insert(list_end(l), element);
}

/// @return Unlinked element for given iterator.
static void *impl_unlink(struct list_iter *it)
{
    void *element;
    struct list_node *source;

    element = list_at(it);
    if (!element) {
        return NULL;
    }

    ///     +-------+                    +-------+
    ///     |       |-1----------------->|       |
    ///     |       |     +--------+     |       |
    /// --->|       |     |    next|-3-X |       |--->
    ///     |  lhs  |     | source |     |  rhs  |
    /// <---|       | X-4-|prev    |     |       |<---
    ///     |       |     +--------+     |       |
    ///     |       |<-----------------2-|       |
    ///     +-------+                    +-------+

    source = &it->node;

    // Unlink from current position.
    source->prev->next = source->next; // 1
    source->next->prev = source->prev; // 2
    source->list->size--;

    // Mark node as unlinked.
    source->next = NULL; // 3
    source->prev = NULL; // 4
    source->list = NULL;

    return element;
}

void *list_pop_front(struct list *l)
{
    return impl_unlink(list_begin(l));
}

void *list_pop_back(struct list *l)
{
    return impl_unlink(list_prev(list_end(l)));
}

void *list_at(struct list_iter *it)
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"

    // Const cast away is safe because @c it is non-const.
    return (void *)list_at_const(it);

#pragma GCC diagnostic pop
}

static const void *impl_element_of(const struct list *l, const struct list_iter *it)
{
    return (const void *)((const char *)it - l->offset);
}

const void *list_at_const(const struct list_iter *it)
{
    const struct list *l;

    if (!it) {
        errno = EINVAL;
        return NULL;
    }

    l = it->node.list;
    if (!l) {
        // Iterator not linked.
        errno = EINVAL;
        return NULL;
    }

    if (it == list_cend(l)) {
        errno = ENOENT;
        return NULL;
    }

    return impl_element_of(l, it);
}

int list_erase(struct list_iter *it, void (*destructor)(void *))
{
    void *element;

    element = impl_unlink(it);
    if (!element) {
        return -1;
    }

    if (destructor) {
        destructor(element);
    }

    return 0;
}

int list_splice(struct list_iter *it, struct list_iter *source_iter)
{
    struct list_node *target;
    struct list_node *source;
    void *element;

    if (!it || !source_iter) {
        errno = EINVAL;
        return -1;
    }

    if (it == source_iter) {
        // Disallow inserting before itself.
        errno = EINVAL;
        return -1;
    }

    source = &source_iter->node;

    if (source_iter == list_end(source->list)) {
        // Disallow list_end() as source.
        errno = ENOENT;
        return -1;
    }

    target = &it->node;

    if (!target->list || !source->list) {
        // Iterator not linked.
        errno = EINVAL;
        return -1;
    }

    if (target->list != source->list) {
        // Disallow splice between different lists.
        errno = EINVAL;
        return -1;
    }

    // Unlink from current position.
    element = impl_unlink(source_iter);

    // Insert source before target.
    list_insert(it, element);

    return 0;
}
