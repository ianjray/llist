#include "llist.h"

#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

struct list {
    // Uses a dummy 'sentinel' node, in order to simplify link management.
    //
    // Base case:
    //           +------+
    //           v      |
    //     +--------+   |
    //     |    next|---+
    //     |sentinel|
    // +---|prev    |
    // |   +--------+
    // |      ^
    // +------+
    //
    // General case:
    //           +--------------------+
    //           v                    |
    //     +--------+    +--------+   |
    //     |    next|--->|    next|---+
    //     |sentinel|    |  node  |
    // +---|prev    |<---|prev    |
    // |   +--------+    +--------+
    // |                    ^
    // +--------------------+
    ///
    struct list_node sentinel;
    size_t size;
    size_t offset;
};

struct list *list_new(size_t offset)
{
    struct list *l = calloc(1, sizeof(struct list));
    if (!l) {
        return NULL;
    }

    l->sentinel.prev = &l->sentinel;
    l->sentinel.next = &l->sentinel;
    l->sentinel.list = l;
    l->offset = offset;
    return l;
}

int list_delete(struct list *l)
{
    if (!l) {
        return -EFAULT;
    }

    free(l);
    return 0;
}

size_t list_size(const struct list *l)
{
    if (!l) {
        return 0;
    }

    return l->size;
}

/// Iterator has the same layout as list_node.
/// Clients only ever see a pointer-to-iterator, thus the implementation is opaque.
struct list_iter {
    struct list_node node;
};

static struct list_iter *impl_begin(struct list *l)
{
    if (!l) {
        return NULL;
    }

    return (struct list_iter *)l->sentinel.next;
}

static struct list_iter *impl_end(struct list *l)
{
    if (!l) {
        return NULL;
    }

    return (struct list_iter *)&l->sentinel;
}

struct list_iter *list_begin(struct list *l)
{
    return impl_begin(l);
}

struct list_iter *list_end(struct list *l)
{
    return impl_end(l);
}

const struct list_iter *list_cbegin(const struct list *l)
{
    // Const cast-away required to use common implementation.
    return impl_begin((struct list *)l);
}

const struct list_iter *list_cend(const struct list *l)
{
    // Const cast-away required to use common implementation.
    return impl_end((struct list *)l);
}

struct list_iter *list_next(struct list_iter *it)
{
    return list_advance(it, 1);
}

struct list_iter *list_prev(struct list_iter *it)
{
    return list_advance(it, -1);
}

const struct list_iter *list_cnext(const struct list_iter *it)
{
    return list_advance_const(it, 1);
}

const struct list_iter *list_cprev(const struct list_iter *it)
{
    return list_advance_const(it, -1);
}

static struct list *impl_list_of_iterator(const struct list_iter *it)
{
    if (!it) {
        return NULL;
    }

    return it->node.list;
}

static const struct list_iter *impl_advance(const struct list_iter *it, ssize_t offset)
{
    struct list *l = impl_list_of_iterator(it);
    if (!l) {
        return NULL;
    }

    while (offset < 0 && it != impl_begin(l)) {
        it = (struct list_iter *)it->node.prev;
        offset++;
    }

    while (offset > 0 && it != impl_end(l)) {
        it = (struct list_iter *)it->node.next;
        offset--;
    }

    return it;
}

struct list_iter *list_advance(struct list_iter *it, ssize_t offset)
{
    // Const cast-away required to use common implementation.
    return (struct list_iter *)impl_advance(it, offset);
}

const struct list_iter *list_advance_const(const struct list_iter *it, ssize_t offset)
{
    return impl_advance(it, offset);
}

static struct list_node *impl_node_of(const struct list *l, void *element)
{
    return (struct list_node *)((char *)element + l->offset);
}

static void *impl_element_of(const struct list_iter *it)
{
    struct list *l = impl_list_of_iterator(it);
    if (!l) {
        return NULL;
    }

    if (it == impl_end(l)) {
        return NULL;
    }

    return (char *)it - l->offset;
}

struct list_iter *list_insert(struct list_iter *it, void *element)
{
    struct list *l;
    struct list_node *link;
    struct list_node *rhs;
    struct list_node *lhs;

    l = impl_list_of_iterator(it);
    if (!l || !element) {
        return NULL;
    }

    link = impl_node_of(l, element);
    if (link->list) {
        // Already in a list.
        return NULL;
    }

    //     +-------+    +--------+    +-------+
    // --->|       |-4->|    next|-1->|       |--->
    //     |  lhs  |    |  link  |    |  rhs  |
    // <---|       |<-2-|prev    |<-3-|       |<---
    //     +-------+    +--------+    +-------+

    rhs = &it->node;
    lhs = rhs->prev;

    link->next = rhs;  // 1
    link->prev = lhs;  // 2
    link->list = l;

    rhs->prev = link;  // 3
    lhs->next = link;  // 4

    l->size++;

    return (struct list_iter *)impl_node_of(l, element);
}

struct list_iter *list_push_front(struct list *l, void *element)
{
    return list_insert(list_begin(l), element);
}

struct list_iter *list_push_back(struct list *l, void *element)
{
    return list_insert(list_end(l), element);
}

void *list_at(struct list_iter *it)
{
    return impl_element_of(it);
}

const void *list_at_const(const struct list_iter *it)
{
    return impl_element_of(it);
}

int list_erase(struct list_iter *it, void (*destructor)(void *))
{
    struct list *l;
    void *element;

    if (!it) {
        return -EFAULT;
    }

    l = impl_list_of_iterator(it);
    if (!l) {
        return -EFAULT;
    }

    element = impl_element_of(it);
    if (!element) {
        return -ENOENT;
    }

    it->node.prev->next = it->node.next;
    it->node.next->prev = it->node.prev;
    l->size--;

    // Mark node as unlinked.
    it->node.list = NULL;
    it->node.next = NULL;
    it->node.prev = NULL;

    if (destructor) {
        destructor(element);
    }

    return 0;
}

int list_erase_range(struct list_iter *it, struct list_iter *end, void (*destructor)(void *))
{
    struct list *l;
    int r;

    if (!it || !end) {
        return -EFAULT;
    }

    l = impl_list_of_iterator(it);
    if (!l) {
        return -EFAULT;
    }

    if (l != impl_list_of_iterator(end)) {
        return -EFAULT;
    }

    while (it != impl_end(l) && it != end) {
        struct list_iter *candidate = it;
        it = list_next(it);

        r = list_erase(candidate, destructor);
        if (r < 0) {
            return r; // UNREACHABLE
        }
    }

    return 0;
}

int list_clear(struct list *l, void (*destructor)(void *))
{
    if (!l) {
        // Clearing a NULL list is not an error.
        return 0;
    }

    return list_erase_range(list_begin(l), list_end(l), destructor);
}

int list_splice(struct list_iter *it, struct list_iter *source_iter)
{
    struct list_node *target;
    struct list_node *source;
    struct list_node *lhs;

    if (!it || !source_iter) {
        return -EFAULT;
    }

    if (it == source_iter) {
        // Disallow inserting before itself.
        return -EINVAL;
    }

    source = &source_iter->node;

    if (source_iter == impl_end(source->list)) {
        // Disallow list_end() as source.
        return -EACCES;
    }

    target = &it->node;

    if (!target->list || !source->list) {
        // Reject iterator that points to an unlinked element.
        return -ENOENT;
    }

    if (target->list != source->list) {
        // Disallow splice between different lists.
        return -EPERM;
    }

    // Unlink source from its current position.
    source->prev->next = source->next;
    source->next->prev = source->prev;

    // Insert source before target.
    lhs = target->prev;
    source->next = target;
    source->prev = lhs;
    target->prev = source;
    lhs->next = source;

    return 0;
}
