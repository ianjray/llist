#include "llist.h"

#include "memory_shim.h"

#include <assert.h>
#include <errno.h>
#include <stdlib.h>

struct node
{
    int n;
    struct list_node link;
};

static struct node *make(void)
{
    return (struct node *)calloc(1, sizeof(struct node));
}

static struct node *make_n(int n)
{
    struct node *t = make();
    t->n = n;
    return t;
}

static void test_list_new(void)
{
    struct list *l;

    g_calloc_fail = true;
    l = list_new(offsetof(struct node, link));
    g_calloc_fail = false;
    assert(NULL == l);
}

static void test_list_delete(void)
{
    list_delete(NULL);
}

static void test_list_size(void)
{
    struct list *l;

    assert(0 == list_size(NULL));

    l = list_new(offsetof(struct node, link));

    assert(0 == list_size(l));

    list_insert(list_begin(l), make_n(1));
    assert(1 == list_size(l));

    list_delete(l);
}

static void test_list_empty(void)
{
    struct list *l;

    assert(list_empty(NULL));

    l = list_new(offsetof(struct node, link));

    assert(list_empty(l));

    list_insert(list_begin(l), make_n(1));
    assert(!list_empty(l));

    list_delete(l);
}

static void test_list_iterator(void)
{
    struct list *l;
    struct node *t;
    struct list_iter *it;
    int calls;

    assert(NULL == list_begin(NULL));
    assert(NULL == list_end(NULL));

    l = list_new(offsetof(struct node, link));

    assert(list_begin(l) == list_end(l));

    list_insert(list_begin(l), make_n(1));
    list_insert(list_begin(l), make_n(2));
    list_insert(list_begin(l), make_n(3));

    assert(list_begin(l) != list_end(l));

    it = list_begin(l);
    t = list_at(it);
    assert(3 == t->n);
    it = list_next(it);

    t = list_at(it);
    assert(2 == t->n);
    it = list_next(it);

    t = list_at(it);
    assert(1 == t->n);
    it = list_next(it);
    assert(it == list_end(l));

    it = list_prev(it);
    t = list_at(it);
    assert(1 == t->n);

    it = list_prev(it);
    t = list_at(it);
    assert(2 == t->n);

    it = list_prev(it);
    t = list_at(it);
    assert(3 == t->n);
    assert(it == list_begin(l));

    list_erase(it, free);

    list_foreach(NULL, it) {
        assert(0);
    }

    calls = 2;
    list_foreach(l, it) {
        struct node *node = list_at(it);
        assert(calls == node->n);
        calls--;
    }

    list_foreach_entry(NULL, struct node, t) {
        assert(0);
    }

    calls = 2;
    list_foreach_entry(l, struct node, t) {
        assert(calls == t->n);
        calls--;
    }

    list_delete(l);
}

static void test_list_iterator_const(void)
{
    struct list *l;
    const struct list *cl;
    const struct list_iter *cit;
    const struct node *ct;
    int calls;

    assert(NULL == list_cbegin(NULL));
    assert(NULL == list_cend(NULL));

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

    list_foreach_const(NULL, it) {
        assert(0);
    }

    calls = 3;
    list_foreach_const(cl, it) {
        const struct node *node = list_at_const(it);
        assert(calls == node->n);
        calls--;
    }

    list_foreach_entry_const(NULL, struct node, ct) {
        assert(0);
    }

    calls = 3;
    list_foreach_entry_const(cl, struct node, ct) {
        assert(calls == ct->n);
        calls--;
    }

    list_delete(l);
}

static void test_list_advance(void)
{
    struct list *l;
    struct list_iter *it;

    assert(NULL == list_advance(NULL, -1));
    assert(NULL == list_advance(NULL, 0));
    assert(NULL == list_advance(NULL, 1));

    l = list_new(offsetof(struct node, link));

    assert(list_begin(l) == list_end(l));

    assert(list_begin(l) == list_advance(list_begin(l), -1));
    assert(list_begin(l) == list_advance(list_begin(l), 0));
    assert(list_begin(l) == list_advance(list_begin(l), 1));

    assert(list_begin(l) == list_advance(list_end(l), -1));
    assert(list_begin(l) == list_advance(list_end(l), 0));
    assert(list_begin(l) == list_advance(list_end(l), 1));

    assert(list_end(l) == list_advance(list_begin(l), -1));
    assert(list_end(l) == list_advance(list_begin(l), 0));
    assert(list_end(l) == list_advance(list_begin(l), 1));

    assert(list_end(l) == list_advance(list_end(l), -1));
    assert(list_end(l) == list_advance(list_end(l), 0));
    assert(list_end(l) == list_advance(list_end(l), 1));

    assert(NULL != list_push_back(l, make_n(1)));
    assert(NULL != list_push_back(l, make_n(2)));
    assert(NULL != list_push_back(l, make_n(3)));

    assert(list_begin(l) != list_end(l));

    it = list_advance(list_begin(l), 0);
    assert(it == list_begin(l));

    it = list_advance(list_begin(l), 3);
    assert(it == list_end(l));

    it = list_advance(list_begin(l), 4);
    assert(it == list_end(l));

    list_delete(l);
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

    cit = list_advance_const(list_cbegin(cl), 4);
    assert(cit == list_cend(cl));

    list_delete(l);
}

static void test_list_insert(void)
{
    struct list *l;
    struct node *t;

    assert(NULL == list_insert(NULL, NULL));

    l = list_new(offsetof(struct node, link));

    assert(NULL == list_insert(list_begin(l), NULL));
    assert(NULL == list_insert(NULL, make_n(0)));

    assert(NULL == list_insert(list_begin(l), NULL));
    assert(NULL == list_insert(list_end(l), NULL));

    t = make_n(1);
    assert(NULL != list_insert(list_end(l), t));
    // 't' already in list.
    assert(NULL == list_insert(list_end(l), t));

    list_delete(l);
}

static void test_list_push_front(void)
{
    struct list *l;

    l = list_new(offsetof(struct node, link));

    assert(NULL != list_push_front(l, make_n(1)));
    assert(NULL != list_push_front(l, make_n(2)));

    assert(((struct node *)list_at(list_advance(list_begin(l), 0)))->n == 2);
    assert(((struct node *)list_at(list_advance(list_begin(l), 1)))->n == 1);

    list_delete(l);
}

static void test_list_push_back(void)
{
    struct list *l;

    l = list_new(offsetof(struct node, link));

    assert(NULL != list_push_back(l, make_n(1)));
    assert(NULL != list_push_back(l, make_n(2)));

    assert(((struct node *)list_at(list_advance(list_begin(l), 0)))->n == 1);
    assert(((struct node *)list_at(list_advance(list_begin(l), 1)))->n == 2);

    list_delete(l);
}

static void test_list_at(void)
{
    struct list *l;
    struct node *t;

    assert(NULL == list_at(NULL));

    l = list_new(offsetof(struct node, link));

    t = make_n(99);

    assert(NULL == list_at((struct list_iter *)(&t->link)));
    assert(NULL == list_at(list_begin(l)));
    assert(NULL == list_at(list_end(l)));

    free(t);

    list_delete(l);
}

static void test_list_erase(void)
{
    struct list *l;
    struct list_iter *it;

    assert(-EFAULT == list_erase(NULL, NULL));

    l = list_new(offsetof(struct node, link));

    assert(-ENOENT == list_erase(list_begin(l), NULL));

    // Erasing list_end() does not make sense.
    assert(-ENOENT == list_erase(list_end(l), NULL));

    assert(NULL != list_push_back(l, make_n(1)));
    assert(NULL != list_push_back(l, make_n(2)));
    assert(NULL != list_push_back(l, make_n(3)));

    it = list_next(list_begin(l));

    // Pass NULL destructor so that double erase can be tested without triggering heap-use-after-free.
    assert(0 == list_erase(it, NULL));

    // Iterator is invalid after erase.
    // An iterator may only be erased once.
    assert(-EFAULT == list_erase(it, free));

    assert(0 == list_erase(list_begin(l), free));
    assert(0 == list_erase(list_begin(l), free));
    assert(-ENOENT == list_erase(list_begin(l), free));

    list_delete(l);
}

static void test_list_erase_range(void)
{
    struct list *l;
    struct list *l2;
    struct node *t;

    l = list_new(offsetof(struct node, link));
    l2 = list_new(offsetof(struct node, link));

    assert(-EFAULT == list_erase_range(NULL, list_next(list_next(list_begin(l))), free));
    assert(-EFAULT == list_erase_range(list_begin(l), NULL, free));
    assert(-EFAULT == list_erase_range(list_begin(l), list_end(l2), free));

    t = make_n(99);
    assert(-EFAULT == list_erase_range((struct list_iter *)(&t->link), list_end(l), free));
    free(t);

    list_insert(list_end(l), make_n(1));
    list_insert(list_end(l), make_n(2));
    list_insert(list_end(l), make_n(3));
    list_insert(list_end(l), make_n(4));

    assert(0 == list_erase_range(list_end(l), list_begin(l), free));
    assert(4 == list_size(l));

    assert(0 == list_erase_range(list_begin(l), list_next(list_next(list_begin(l))), free));
    assert(2 == list_size(l));
    assert(((struct node *)list_at(list_advance(list_begin(l), 0)))->n == 3);
    assert(((struct node *)list_at(list_advance(list_begin(l), 1)))->n == 4);

    list_delete(l);
    list_delete(l2);
}

static void test_list_clear(void)
{
    assert(0 == list_clear(NULL, NULL));
}

static void test_list_splice(void)
{
    struct list *l;
    struct list *l2;
    struct node *t;

    l = list_new(offsetof(struct node, link));
    t = list_at(list_insert(list_end(l), make_n(1)));
    t = list_at(list_insert(list_end(l), make_n(2)));
    t = list_at(list_insert(list_end(l), make_n(3)));
    assert(((struct node *)list_at(list_advance(list_begin(l), 0)))->n == 1);
    assert(((struct node *)list_at(list_advance(list_begin(l), 1)))->n == 2);
    assert(((struct node *)list_at(list_advance(list_begin(l), 2)))->n == 3);


    assert(-EFAULT == list_splice(NULL, NULL));
    assert(-EFAULT == list_splice(list_begin(l), NULL));
    assert(-EFAULT == list_splice(NULL, list_begin(l)));

    // Source list_end() not allowed.
    assert(-EACCES == list_splice(list_begin(l), list_end(l)));

    // Source must be in a list.
    t = make();
    assert(-ENOENT == list_splice(list_begin(l), (struct list_iter *)(&t->link)));
    free(t);

    // Source must be in the same list.
    l2 = list_new(offsetof(struct node, link));
    list_insert(list_end(l2), make());
    assert(-EPERM == list_splice(list_begin(l), list_begin(l2)));
    list_clear(l2, free);
    list_delete(l2);

    list_splice(list_begin(l), list_prev(list_end(l)));
    assert(((struct node *)list_at(list_advance(list_begin(l), 0)))->n == 3);
    assert(((struct node *)list_at(list_advance(list_begin(l), 1)))->n == 1);
    assert(((struct node *)list_at(list_advance(list_begin(l), 2)))->n == 2);

    list_splice(list_end(l), list_begin(l));
    assert(((struct node *)list_at(list_advance(list_begin(l), 0)))->n == 1);
    assert(((struct node *)list_at(list_advance(list_begin(l), 1)))->n == 2);
    assert(((struct node *)list_at(list_advance(list_begin(l), 2)))->n == 3);

    list_splice(list_begin(l), list_begin(l));
    assert(((struct node *)list_at(list_advance(list_begin(l), 0)))->n == 1);
    assert(((struct node *)list_at(list_advance(list_begin(l), 1)))->n == 2);
    assert(((struct node *)list_at(list_advance(list_begin(l), 2)))->n == 3);

    list_delete(l);
}

int main(void)
{
    test_list_new();
    test_list_delete();
    test_list_size();
    test_list_empty();
    test_list_iterator();
    test_list_iterator_const();
    test_list_advance();
    test_list_advance_const();
    test_list_insert();
    test_list_push_front();
    test_list_push_back();
    test_list_at();
    test_list_erase();
    test_list_erase_range();
    test_list_clear();
    test_list_splice();
    return 0;
}
