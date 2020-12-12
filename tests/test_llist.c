#include "llist.h"

#include "memory_shim.h"

#include <assert.h>
#include <errno.h>
#include <stdlib.h>

#include "llist.c"

struct node
{
    int n;
    LIST_NODE(link);
};

static struct node *make(void)
{
    return calloc(1, sizeof(struct node));
}

static struct node *make_n(int n)
{
    struct node *node = make();
    node->n = n;
    return node;
}

static void test_list_new(void)
{
    struct list *l;

    // Invalid offset.
    errno = 0;
    l = list_new(1);
    assert(NULL == l);
    assert(EINVAL == errno);

    memory_shim_fail_at(1);
    errno = 0;
    l = list_new(offsetof(struct node, link));
    memory_shim_reset();
    assert(NULL == l);
    assert(ENOMEM == errno);
}

static void test_list_delete(void)
{
    list_delete(NULL, NULL);
}

static void test_list_empty(void)
{
    struct list *l;

    assert(list_empty(NULL));

    l = list_new(offsetof(struct node, link));

    assert(list_empty(l));

    list_insert(list_begin(l), make_n(1));
    assert(!list_empty(l));

    list_delete(l, free);
}

static void test_list_size(void)
{
    struct list *l;

    assert(0 == list_size(NULL));

    l = list_new(offsetof(struct node, link));

    assert(0 == list_size(l));

    list_insert(list_begin(l), make_n(1));
    assert(1 == list_size(l));

    list_delete(l, free);
}

static void test_list_clear(void)
{
    struct list *l;

    errno = 0;
    assert(-1 == list_clear(NULL, NULL));
    assert(EINVAL == errno);

    l = list_new(offsetof(struct node, link));

    assert(0 == list_clear(l, NULL));
    assert(0 == list_clear(l, NULL));

    list_delete(l, free);
}

static void test_list_iterator(void)
{
    struct list *l;
    struct node *n;
    struct list_iter *it;

    errno = 0;
    assert(NULL == list_begin(NULL));
    assert(EINVAL == errno);

    errno = 0;
    assert(NULL == list_end(NULL));
    assert(EINVAL == errno);

    errno = 0;
    assert(NULL == list_next(NULL));
    assert(EINVAL == errno);

    errno = 0;
    assert(NULL == list_prev(NULL));
    assert(EINVAL == errno);

    l = list_new(offsetof(struct node, link));

    assert(list_begin(l) == list_end(l));

    list_insert(list_begin(l), make_n(1));
    list_insert(list_begin(l), make_n(2));
    list_insert(list_begin(l), make_n(3));

    assert(list_begin(l) != list_end(l));

    it = list_begin(l);
    n = list_at(it);
    assert(3 == n->n);
    it = list_next(it);

    n = list_at(it);
    assert(2 == n->n);
    it = list_next(it);

    n = list_at(it);
    assert(1 == n->n);
    it = list_next(it);
    assert(it == list_end(l));

    it = list_prev(it);
    n = list_at(it);
    assert(1 == n->n);

    it = list_prev(it);
    n = list_at(it);
    assert(2 == n->n);

    it = list_prev(it);
    n = list_at(it);
    assert(3 == n->n);
    assert(it == list_begin(l));

    assert(NULL == list_at(list_end(l)));

    list_delete(l, free);
}

static void test_list_iterator_const(void)
{
    struct list *l;
    const struct list *cl;
    const struct list_iter *cit;
    const struct node *ct;

    assert(NULL == list_cbegin(NULL));
    assert(NULL == list_cend(NULL));

    assert(NULL == list_cnext(NULL));
    assert(NULL == list_cprev(NULL));

    l = list_new(offsetof(struct node, link));

    assert(list_cbegin(l) == list_cend(l));

    list_insert(list_begin(l), make_n(1));
    list_insert(list_begin(l), make_n(2));
    list_insert(list_begin(l), make_n(3));

    assert(list_cbegin(l) != list_cend(l));

    cl = l;

    cit = list_cbegin(cl);
    ct = list_at_const(cit);
    assert(3 == ct->n);
    cit = list_cnext(cit);

    ct = list_at_const(cit);
    assert(2 == ct->n);
    cit = list_cnext(cit);

    ct = list_at_const(cit);
    assert(1 == ct->n);
    cit = list_cnext(cit);
    assert(cit == list_cend(cl));

    cit = list_cprev(cit);
    ct = list_at_const(cit);
    assert(1 == ct->n);

    cit = list_cprev(cit);
    ct = list_at_const(cit);
    assert(2 == ct->n);

    cit = list_cprev(cit);
    ct = list_at_const(cit);
    assert(3 == ct->n);
    assert(cit == list_cbegin(cl));

    assert(NULL == list_at_const(list_cend(cl)));

    list_delete(l, free);
}

static void test_list_advance(void)
{
    struct list *l;
    struct list_iter *it;
    struct node *tmp;

    assert(NULL == list_advance(NULL, -1));
    assert(NULL == list_advance(NULL, 0));
    assert(NULL == list_advance(NULL, 1));

    l = list_new(offsetof(struct node, link));

    assert(list_begin(l) == list_end(l));

    errno = 0;
    assert(NULL == list_advance(list_begin(l), -1));
    assert(ERANGE == errno);

    assert(list_begin(l) == list_advance(list_begin(l), 0));

    errno = 0;
    assert(NULL == list_advance(list_begin(l), 1));
    assert(ERANGE == errno);

    errno = 0;
    assert(NULL == list_advance(list_end(l), -1));
    assert(ERANGE == errno);

    assert(list_end(l) == list_advance(list_end(l), 0));

    errno = 0;
    assert(NULL == list_advance(list_end(l), 1));
    assert(ERANGE == errno);

    assert(NULL != list_push_back(l, make_n(1)));
    assert(NULL != list_push_back(l, make_n(2)));
    assert(NULL != list_push_back(l, make_n(3)));

    assert(list_begin(l) != list_end(l));

    it = list_advance(list_begin(l), 0);
    assert(it == list_begin(l));

    it = list_advance(list_begin(l), 3);
    assert(it == list_end(l));

    errno = 0;
    it = list_advance(list_begin(l), 4);
    assert(NULL == it);
    assert(ERANGE == errno);

    // Invalid iterator.
    tmp = make_n(4);
    it = list_insert(list_end(l), tmp);
    list_erase(it, NULL);
    errno = 0;
    assert(NULL == list_advance(it, 0));
    assert(EINVAL == errno);
    free(tmp);

    list_delete(l, free);
}

static void test_list_advance_const(void)
{
    struct list *l;
    const struct list *cl;
    const struct list_iter *cit;

    assert(NULL == list_advance_const(NULL, -1));
    assert(NULL == list_advance_const(NULL, 0));
    assert(NULL == list_advance_const(NULL, 1));

    l = list_new(offsetof(struct node, link));

    assert(list_cbegin(l) == list_cend(l));

    list_insert(list_begin(l), make_n(1));
    list_insert(list_begin(l), make_n(2));
    list_insert(list_begin(l), make_n(3));

    assert(list_cbegin(l) != list_cend(l));

    cl = l;

    cit = list_advance_const(list_cbegin(cl), 0);
    assert(cit == list_cbegin(cl));

    cit = list_advance_const(list_cbegin(cl), 3);
    assert(cit == list_cend(cl));

    errno = 0;
    cit = list_advance_const(list_cbegin(cl), 4);
    assert(NULL == cit);
    assert(ERANGE == errno);

    list_delete(l, free);
}

static void test_list_element(void)
{
    struct list *l;
    struct list_iter *it;
    struct node *n;
    const struct list_iter *cit;
    const struct node *cn;

    errno = 0;
    assert(NULL == list_element(NULL, 0));
    assert(EINVAL == errno);

    l = list_new(offsetof(struct node, link));

    n = make_n(1);
    list_insert(list_end(l), n);

    // Invalid offset.
    errno = 0;
    assert(NULL == list_element(n, 1));
    assert(EINVAL == errno);

    it = list_element(n, offsetof(struct node, link));
    assert(it == list_begin(l));

    cn = n;
    cit = list_element_const(cn, offsetof(struct node, link));
    assert(cit == list_cbegin(l));

    list_delete(l, free);
}

static void test_list_insert(void)
{
    struct list *l;
    struct list_iter *it;
    struct node *n;
    struct node *tmp;

    assert(NULL == list_insert(NULL, NULL));

    l = list_new(offsetof(struct node, link));

    assert(NULL == list_insert(list_begin(l), NULL));
    n = make_n(0);
    assert(NULL == list_insert(NULL, n));
    free(n);

    assert(NULL == list_insert(list_begin(l), NULL));
    assert(NULL == list_insert(list_end(l), NULL));

    // Invalid iterator.
    tmp = make_n(2);
    it = list_insert(list_end(l), tmp);
    list_erase(it, NULL);
    n = make_n(3);
    errno = 0;
    assert(NULL == list_insert(it, n));
    assert(EINVAL == errno);
    free(n);
    free(tmp);

    // Simulate size overflow.
    l->size = SIZE_MAX;
    n = make_n(1);
    errno = 0;
    assert(NULL == list_insert(list_end(l), n));
    assert(EOVERFLOW == errno);
    free(n);
    l->size = 0;

    // Simulate offset corrupt.
    l->offset = 1;
    n = make_n(1);
    errno = 0;
    assert(NULL == list_insert(list_end(l), n));
    assert(EINVAL == errno);
    free(n);
    l->offset = offsetof(struct node, link);

    assert(NULL != list_insert(list_end(l), make_n(1)));

    list_delete(l, free);
}

static void test_list_push_front(void)
{
    struct list *l;
    struct node *n;

    n = make_n(0);
    errno = 0;
    assert(NULL == list_push_front(NULL, n));
    assert(EINVAL == errno);
    free(n);

    l = list_new(offsetof(struct node, link));

    assert(NULL != list_push_front(l, make_n(1)));
    assert(NULL != list_push_front(l, make_n(2)));

    assert(((struct node *)list_at(list_advance(list_begin(l), 0)))->n == 2);
    assert(((struct node *)list_at(list_advance(list_begin(l), 1)))->n == 1);

    list_delete(l, free);
}

static void test_list_push_back(void)
{
    struct list *l;
    struct node *n;

    n = make_n(0);
    errno = 0;
    assert(NULL == list_push_back(NULL, n));
    assert(EINVAL == errno);
    free(n);

    l = list_new(offsetof(struct node, link));

    assert(NULL != list_push_back(l, make_n(1)));
    assert(NULL != list_push_back(l, make_n(2)));

    assert(((struct node *)list_at(list_advance(list_begin(l), 0)))->n == 1);
    assert(((struct node *)list_at(list_advance(list_begin(l), 1)))->n == 2);

    list_delete(l, free);
}

static void test_list_pop_front(void)
{
    struct list *l;
    struct node *n;
    struct node *m;

    errno = 0;
    assert(NULL == list_pop_front(NULL));
    assert(EINVAL == errno);

    l = list_new(offsetof(struct node, link));

    // Empty.
    errno = 0;
    assert(NULL == list_pop_front(l));
    assert(ENOENT == errno);

    n = make_n(1);
    assert(NULL != list_push_back(l, n));
    assert(NULL != list_push_back(l, make_n(2)));
    assert(2 == list_size(l));

    m = list_pop_front(l);
    assert(m == n);
    free(n);

    assert(1 == list_size(l));
    assert(((struct node *)list_at(list_begin(l)))->n == 2);

    list_delete(l, free);
}

static void test_list_pop_back(void)
{
    struct list *l;
    struct node *n;
    struct node *m;

    errno = 0;
    assert(NULL == list_pop_back(NULL));
    assert(EINVAL == errno);

    l = list_new(offsetof(struct node, link));

    // Empty.
    errno = 0;
    assert(NULL == list_pop_back(l));
    assert(EINVAL == errno);

    assert(NULL != list_push_back(l, make_n(1)));
    n = make_n(2);
    assert(NULL != list_push_back(l, n));
    assert(2 == list_size(l));

    m = list_pop_back(l);
    assert(m == n);
    free(n);

    assert(1 == list_size(l));
    assert(((struct node *)list_at(list_begin(l)))->n == 1);

    list_delete(l, free);
}

static void test_list_at(void)
{
    struct list *l;
    struct node *n;

    errno = 0;
    assert(NULL == list_at(NULL));
    assert(EINVAL == errno);

    errno = 0;
    assert(NULL == list_at_const(NULL));
    assert(EINVAL == errno);

    l = list_new(offsetof(struct node, link));

    n = make_n(99);

    assert(NULL == list_at((struct list_iter *)(&n->link)));
    assert(EINVAL == errno);

    errno = 0;
    assert(NULL == list_at(list_begin(l)));
    assert(ENOENT == errno);

    errno = 0;
    assert(NULL == list_at(list_end(l)));
    assert(ENOENT == errno);

    free(n);

    list_delete(l, free);
}

static void test_list_erase(void)
{
    struct list *l;
    struct list_iter *it;
    struct node *n;

    errno = 0;
    assert(-1 == list_erase(NULL, NULL));
    assert(EINVAL == errno);

    l = list_new(offsetof(struct node, link));

    errno = 0;
    assert(-1 == list_erase(list_begin(l), NULL));
    assert(ENOENT == errno);

    assert(NULL != list_push_back(l, make_n(1)));
    assert(NULL != list_push_back(l, make_n(2)));
    assert(NULL != list_push_back(l, make_n(3)));

    // Erasing list_end() does not make sense.
    assert(3 == list_size(l));
    errno = 0;
    assert(-1 == list_erase(list_end(l), NULL));
    assert(ENOENT == errno);
    assert(3 == list_size(l));

    it = list_next(list_begin(l));
    n = list_at(it);

    // Pass NULL destructor so that double erase can be tested without triggering heap-use-after-free.
    assert(0 == list_erase(it, NULL));

    // Iterator is invalid after erase.
    // An iterator may only be erased once.
    errno = 0;
    assert(-1 == list_erase(it, free));
    assert(EINVAL == errno);
    free(n);

    assert(0 == list_erase(list_begin(l), free));
    assert(0 == list_erase(list_begin(l), free));
    errno = 0;
    assert(-1 == list_erase(list_begin(l), free));
    assert(ENOENT == errno);

    list_delete(l, free);
}

static void test_list_splice(void)
{
    struct list *l;
    struct list *l2;
    struct node *n;
    struct list_iter *dest;
    struct list_iter *source;

    l = list_new(offsetof(struct node, link));
    n = list_at(list_insert(list_end(l), make_n(1)));
    n = list_at(list_insert(list_end(l), make_n(2)));
    n = list_at(list_insert(list_end(l), make_n(3)));
    assert(((struct node *)list_at(list_advance(list_begin(l), 0)))->n == 1);
    assert(((struct node *)list_at(list_advance(list_begin(l), 1)))->n == 2);
    assert(((struct node *)list_at(list_advance(list_begin(l), 2)))->n == 3);

    errno = 0;
    assert(-1 == list_splice(NULL, NULL));
    assert(EINVAL == errno);

    errno = 0;
    assert(-1 == list_splice(list_begin(l), NULL));
    assert(EINVAL == errno);

    errno = 0;
    assert(-1 == list_splice(NULL, list_begin(l)));
    assert(EINVAL == errno);

    // Source list_end() not allowed.
    errno = 0;
    assert(-1 == list_splice(list_begin(l), list_end(l)));
    assert(ENOENT == errno);

    // Source must be in a list.
    n = make();
    errno = 0;
    assert(-1 == list_splice(list_begin(l), (struct list_iter *)(&n->link)));
    assert(EINVAL == errno);
    free(n);

    // Source must be in the same list.
    l2 = list_new(offsetof(struct node, link));
    list_insert(list_end(l2), make());
    errno = 0;
    assert(-1 == list_splice(list_begin(l), list_begin(l2)));
    assert(EINVAL == errno);
    list_delete(l2, free);

    dest = list_begin(l);
    source = list_prev(list_end(l));
    assert(list_next(source) == list_end(l));
    assert(((struct node *)list_at(dest))->n == 1);
    assert(((struct node *)list_at(source))->n == 3);

    assert(0 == list_splice(dest, source));
    // The iterators remain valid, and point to the same elements.
    assert(dest == list_next(list_begin(l)));
    assert(source == list_begin(l));
    assert(((struct node *)list_at(dest))->n == 1);
    assert(((struct node *)list_at(source))->n == 3);

    assert(((struct node *)list_at(list_advance(list_begin(l), 0)))->n == 3);
    assert(((struct node *)list_at(list_advance(list_begin(l), 1)))->n == 1);
    assert(((struct node *)list_at(list_advance(list_begin(l), 2)))->n == 2);

    assert(0 == list_splice(list_end(l), list_begin(l)));
    assert(((struct node *)list_at(list_advance(list_begin(l), 0)))->n == 1);
    assert(((struct node *)list_at(list_advance(list_begin(l), 1)))->n == 2);
    assert(((struct node *)list_at(list_advance(list_begin(l), 2)))->n == 3);

    // Disallow splicing an element to right before itself.
    errno = 0;
    assert(-1 == list_splice(list_begin(l), list_begin(l)));
    assert(EINVAL == errno);
    assert(((struct node *)list_at(list_advance(list_begin(l), 0)))->n == 1);
    assert(((struct node *)list_at(list_advance(list_begin(l), 1)))->n == 2);
    assert(((struct node *)list_at(list_advance(list_begin(l), 2)))->n == 3);

    // Splice an element to right after itself is a no-op.
    assert(0 == list_splice(list_next(list_begin(l)), list_begin(l)));
    assert(((struct node *)list_at(list_advance(list_begin(l), 0)))->n == 1);
    assert(((struct node *)list_at(list_advance(list_begin(l), 1)))->n == 2);
    assert(((struct node *)list_at(list_advance(list_begin(l), 2)))->n == 3);

    list_delete(l, free);
}

static void test_stress(void)
{
    struct list *l;
    size_t n;

    l = list_new(offsetof(struct node, link));

    n = 0;
    for (int i = 1; i <= 1000; i++) {
        list_push_back(l, make_n(i));
        n++;
        assert(n == list_size(l));

        if (i % 3 == 0) {
            list_erase(list_begin(l), free);
            n--;
            assert(n == list_size(l));
        }
    }

    list_delete(l, free);
}

int main(void)
{
    test_list_new();
    test_list_delete();
    test_list_empty();
    test_list_size();
    test_list_clear();
    test_list_iterator();
    test_list_iterator_const();
    test_list_advance();
    test_list_advance_const();
    test_list_element();
    test_list_insert();
    test_list_push_front();
    test_list_push_back();
    test_list_pop_front();
    test_list_pop_back();
    test_list_at();
    test_list_erase();
    test_list_splice();
    test_stress();
    return 0;
}
