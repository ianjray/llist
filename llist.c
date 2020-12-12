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
    if (l->size > 0) {
        return &l->sentinel.next->userdata;
    }
    return NULL;
}

void *list_back(struct list *l)
{
    if (l->size > 1) {
        return &l->sentinel.prev->userdata;
    }
    return NULL;
}

struct list_iter *list_begin(struct list *l)
{
    return (struct list_iter *)l->sentinel.next;
}

struct list_iter *list_end(struct list *l)
{
    return (struct list_iter *)&l->sentinel;
}

void *list_iter_deref(struct list_iter *iter)
{
    if (iter == list_end(iter->node.list)) {
        return NULL;
    }
    return &iter->node.userdata;
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
    struct list *l = iter->node.list;
    while (distance < 0) {
        if (iter == list_begin(l)) {
            break;
        } else {
            iter = (struct list_iter *)iter->node.prev;
            distance++;
        }
    }
    while (distance > 0) {
        iter = (struct list_iter *)iter->node.next;
        distance--;
        if (iter == list_end(l)) {
            break;
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
    if (l && node) {
        node->prev->next = node->next;
        node->next->prev = node->prev;
        impl_release(l, node);
        l->size--;
    }
}

void list_clear(struct list *l)
{
    while (!list_empty(l)) {
        list_erase(l, list_begin(l));
    }
}

static void *impl_insert_before(struct list *l, struct list_iter *it)
{
    struct list_node *other = (struct list_node *)it;
    struct list_node *node = mem.alloc(l->element_size);
    struct list_node *previous = other->prev;

    /*
     *     +--------+    +--------+    +--------+
     * --->|        |-4->|    next|-1->|        |--->
     *     |previous|    |  node  |    | other  |
     * <---|        |<-2-|prev    |<-3-|        |<---
     *     +--------+    +--------+    +--------+
     */
    node->next     = other;     // 1
    node->prev     = previous;  // 2
    other->prev    = node;      // 3
    previous->next = node;      // 4

    node->list = l;
    l->size++;
    return &node->userdata;
}

void *list_insert(struct list *l, struct list_iter *it)
{
    return impl_insert_before(l, it);
}

void list_erase(struct list *l, struct list_iter *it)
{
    if (it != list_end(l)) {
        impl_unlink(l, (struct list_node *)it);
    }
}

void *list_push_back(struct list *l)
{
    return impl_insert_before(l, list_end(l));
}

void list_pop_back(struct list *l)
{
    if (l->sentinel.prev != &l->sentinel) {
        impl_unlink(l, l->sentinel.prev);
    }
}

void *list_push_front(struct list *l)
{
    return impl_insert_before(l, list_begin(l));
}

void list_pop_front(struct list *l)
{
    if (l->sentinel.next != &l->sentinel) {
        impl_unlink(l, l->sentinel.next);
    }
}


#ifdef UNITTEST_LLIST

#include <assert.h>

struct test {
    size_t a;
};

static void test_destructor(void *t)
{
    assert(t != NULL);
}

//@unittest clang -g -fsanitize=address -fsanitize=undefined-trap -fsanitize-undefined-trap-on-error -Weverything -Werror -Wno-padded -Wno-poison-system-directories -DUNITTEST_LLIST
int main(void)
{
    assert(list_empty(NULL));
    assert(list_size(NULL) == 0);

    struct list *l;
    struct list_iter *it;
    struct test *t1, *t2, *t3, *t4, *t5, *t6;

    l = list_new(sizeof(struct test), test_destructor);
    assert(list_empty(l));
    assert(list_size(l) == 0);
    assert(NULL == list_front(l));
    assert(NULL == list_back(l));

    it = list_begin(l);
    assert(it == list_end(l));

    list_clear(l);
    assert(list_size(l) == 0);

    t1 = list_push_back(l);
    t1->a = 1;
    assert(!list_empty(l));
    assert(list_size(l) == 1);
    assert(t1 == list_front(l));
    assert(NULL == list_back(l));
    assert(((struct test *)list_iter_deref(list_iter_advance(list_begin(l), 0)))->a == 1);

    it = list_begin(l);
    assert(it != list_end(l));
    assert(t1 == list_iter_deref(it));
    it = list_iter_advance(it, 0);
    assert(t1 == list_iter_deref(it));
    it = list_iter_advance(it, 99);
    assert(it == list_end(l));

    t2 = list_push_back(l);
    t2->a = 2;
    assert(list_size(l) == 2);
    assert(t1 == list_front(l));
    assert(t2 == list_back(l));
    assert(((struct test *)list_iter_deref(list_iter_advance(list_begin(l), 0)))->a == 1);
    assert(((struct test *)list_iter_deref(list_iter_advance(list_begin(l), 1)))->a == 2);

    it = list_begin(l);
    assert(it != list_end(l));
    assert(t1 == list_iter_deref(it));

    t3 = list_push_front(l);
    t3->a = 3;
    assert(list_size(l) == 3);
    assert(t3 == list_front(l));
    assert(t1 == list_iter_deref(list_iter_advance(list_begin(l), 1)));
    assert(t2 == list_back(l));
    assert(((struct test *)list_iter_deref(list_iter_advance(list_begin(l), 0)))->a == 3);
    assert(((struct test *)list_iter_deref(list_iter_advance(list_begin(l), 1)))->a == 1);
    assert(((struct test *)list_iter_deref(list_iter_advance(list_begin(l), 2)))->a == 2);

    it = list_begin(l);
    assert(t3 == list_iter_deref(it));
    it = list_iter_inc(it);
    assert(t1 == list_iter_deref(it));
    it = list_iter_inc(it);
    assert(t2 == list_iter_deref(it));
    it = list_iter_inc(it);
    assert(list_end(l) == it);

    it = list_iter_dec(it);
    assert(t2 == list_iter_deref(it));
    it = list_iter_advance(it, -2);
    assert(t3 == list_iter_deref(it));
    it = list_iter_advance(it, -99);
    assert(it == list_begin(l));

    list_pop_back(l);
    assert(list_size(l) == 2);
    assert(t3 == list_front(l));
    assert(t1 == list_back(l));
    assert(t3 == list_iter_deref(            list_begin(l)));
    assert(t1 == list_iter_deref(list_iter_dec(list_end(l))));

    list_pop_front(l);
    assert(list_size(l) == 1);
    assert(t1 == list_front(l));
    assert(NULL == list_back(l));
    assert(t1 == list_iter_deref(            list_begin(l)));
    assert(t1 == list_iter_deref(list_iter_dec(list_end(l))));

    list_clear(l);
    assert(list_size(l) == 0);

    t1 = list_push_back(l);
    t1->a = 11;
    assert(list_size(l) == 1);

    t4 = list_insert(l, list_begin(l));
    t4->a = 4;
    assert(list_size(l) == 2);
    t5 = list_insert(l, list_end(l));
    t5->a = 5;
    assert(list_size(l) == 3);
    t6 = list_insert(l, list_begin(l));
    t6->a = 6;
    assert(list_size(l) == 4);
    assert(((struct test *)list_iter_deref(list_iter_advance(list_begin(l), 0)))->a == 6);
    assert(((struct test *)list_iter_deref(list_iter_advance(list_begin(l), 1)))->a == 4);
    assert(((struct test *)list_iter_deref(list_iter_advance(list_begin(l), 2)))->a == 11);
    assert(((struct test *)list_iter_deref(list_iter_advance(list_begin(l), 3)))->a == 5);

    assert(list_iter_deref(list_end(l)) == NULL);

    list_erase(l, list_begin(l));
    assert(list_size(l) == 3);

    list_erase(l, list_iter_inc(list_begin(l)));
    assert(list_size(l) == 2);
    assert(t4 == list_front(l));
    assert(t5 == list_back(l));

    list_delete(l);
    l = NULL;

    list_allocator_set(malloc, free);

    l = list_new(sizeof(struct test), test_destructor);
    list_delete(l);
    l = NULL;
}

#endif
