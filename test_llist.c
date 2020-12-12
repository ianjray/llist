#include "llist.h"

#include <assert.h>
#include <stdlib.h>


struct test
{
    size_t a;
};

static void test_destructor(void *t)
{
    assert(t != NULL);
}

int main(void)
{
    struct list *l, *l2;
    struct list_iter *it;
    struct test *t1, *t2, *t3, *t4, *t5, *t6;

    assert(list_empty(NULL));
    assert(list_size(NULL) == 0);

    l = list_new(sizeof(struct test), test_destructor);
    assert(list_empty(l));
    assert(list_size(l) == 0);
    assert(NULL == list_front(l));
    assert(NULL == list_back(l));

    assert(NULL == list_begin(NULL));
    assert(NULL == list_end(NULL));

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

    it = list_begin(l);
    assert(it != list_end(l));
    assert(t1 == list_iter_deref(it));
    assert(it == list_iter_advance(it, 0));
    it = list_iter_advance(it, 99);
    assert(it == list_end(l));

    t2 = list_push_back(l);
    t2->a = 2;
    assert(list_size(l) == 2);
    assert(t1 == list_front(l));
    assert(t2 == list_back(l));
    assert(((struct test *)list_iter_deref(                  list_begin(l)    ))->a == 1);
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
    it = list_iter_inc(it);
    assert(list_end(l) == it);

    it = list_iter_dec(it);
    assert(t2 == list_iter_deref(it));
    it = list_iter_advance(it, -2);
    assert(t3 == list_iter_deref(it));
    it = list_iter_advance(it, -99);
    assert(it == list_begin(l));

    it = list_end(l);
    it = list_iter_dec(it);
    assert(t2 == list_iter_deref(it));
    it = list_iter_dec(it);
    assert(t1 == list_iter_deref(it));
    it = list_iter_dec(it);
    assert(t3 == list_iter_deref(it));
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

    assert(NULL == list_insert(NULL, list_begin(l)));

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

    l2 = list_new(sizeof(struct test), test_destructor);
    list_erase(l, list_begin(l2));
    assert(list_size(l) == 4);
    list_delete(l2);
    l2 = NULL;

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

    l = list_new(sizeof(struct test), test_destructor);
    t1 = list_push_back(l);
    t1->a = 1;
    t1 = list_push_back(l);
    t1->a = 2;
    t1 = list_push_back(l);
    t1->a = 3;
    assert(((struct test *)list_iter_deref(list_iter_advance(list_begin(l), 0)))->a == 1);
    assert(((struct test *)list_iter_deref(list_iter_advance(list_begin(l), 1)))->a == 2);
    assert(((struct test *)list_iter_deref(list_iter_advance(list_begin(l), 2)))->a == 3);

    list_splice(l, list_begin(l), list_iter_dec(list_end(l)));
    assert(((struct test *)list_iter_deref(list_iter_advance(list_begin(l), 0)))->a == 3);
    assert(((struct test *)list_iter_deref(list_iter_advance(list_begin(l), 1)))->a == 1);
    assert(((struct test *)list_iter_deref(list_iter_advance(list_begin(l), 2)))->a == 2);

    list_splice(l, list_end(l), list_begin(l));
    assert(((struct test *)list_iter_deref(list_iter_advance(list_begin(l), 0)))->a == 1);
    assert(((struct test *)list_iter_deref(list_iter_advance(list_begin(l), 1)))->a == 2);
    assert(((struct test *)list_iter_deref(list_iter_advance(list_begin(l), 2)))->a == 3);

    list_splice(l, list_begin(l), list_begin(l));
    assert(((struct test *)list_iter_deref(list_iter_advance(list_begin(l), 0)))->a == 1);
    assert(((struct test *)list_iter_deref(list_iter_advance(list_begin(l), 1)))->a == 2);
    assert(((struct test *)list_iter_deref(list_iter_advance(list_begin(l), 2)))->a == 3);

    l2 = list_new(sizeof(struct test), test_destructor);

    t1 = list_push_back(l2);
    t1->a = 4;

    list_splice(l, list_end(l2), list_begin(l));
    assert(((struct test *)list_iter_deref(list_iter_advance(list_begin(l), 0)))->a == 1);
    assert(((struct test *)list_iter_deref(list_iter_advance(list_begin(l), 1)))->a == 2);
    assert(((struct test *)list_iter_deref(list_iter_advance(list_begin(l), 2)))->a == 3);

    list_splice(l, list_end(l), list_begin(l2));
    assert(((struct test *)list_iter_deref(list_iter_advance(list_begin(l), 0)))->a == 1);
    assert(((struct test *)list_iter_deref(list_iter_advance(list_begin(l), 1)))->a == 2);
    assert(((struct test *)list_iter_deref(list_iter_advance(list_begin(l), 2)))->a == 3);

    list_delete(l2);
    l2 = NULL;

    list_delete(l);
    l = NULL;
}
