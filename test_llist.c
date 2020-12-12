#include "llist.h"

#include <assert.h>
#include <stdlib.h>

struct test
{
    size_t a;
    struct list_node link;
};

static struct test *make(void)
{
    return (struct test *)calloc(1, sizeof(struct test));
}

int main(void)
{
    struct list *l;
    struct test *t1, *t2, *t3, *t4;

    assert(0    == list_size(NULL));
    assert(NULL == list_begin(NULL));
    assert(NULL == list_end(NULL));
    assert(NULL == list_iter_move(NULL, 0));
    assert(NULL == list_insert(NULL, NULL));
    assert(NULL == list_unlink(NULL));
    assert(NULL == list_at(NULL));

    l = list_new(offsetof(struct test, link));
    assert(NULL != l);
    assert(0    == list_size(l));
    assert(list_begin(l) == list_end(l));
    assert(list_begin(l) == list_iter_move(list_begin(l), 0));
    assert(list_end(l)   == list_iter_move(list_begin(l), 1));
    assert(list_end(l)   == list_iter_move(list_begin(l), 100));
    assert(list_end(l)   == list_iter_move(list_end(l), 0));
    assert(list_begin(l) == list_iter_move(list_end(l), -1));
    assert(list_begin(l) == list_iter_move(list_end(l), -100));
    assert(NULL == list_insert(list_begin(l), NULL));
    assert(NULL == list_insert(list_end(l), NULL));
    assert(NULL == list_unlink(list_begin(l)));
    assert(NULL == list_unlink(list_end(l)));
    assert(NULL == list_at(list_begin(l)));
    assert(NULL == list_at(list_end(l)));

    t2 = list_insert(list_begin(l), make());
    assert(NULL != t2);
    t2->a = 2;
    assert(1 == list_size(l));
    assert(list_begin(l) != list_end(l));
    assert(list_end(l)   == list_next(list_begin(l)));
    assert(list_begin(l) == list_prev(list_prev(list_end(l))));
    assert(t2 == list_at(list_begin(l)));
    assert(t2 == list_at(list_prev(list_end(l))));
    assert(t2 == list_at_offset(list_begin(l), 0));
    assert(t2 == list_at_offset(list_end(l), -1));

    t1 = list_insert(list_begin(l), make());
    assert(NULL != t1);
    t1->a = 1;
    assert(2 == list_size(l));
    assert(list_begin(l) != list_end(l));
    assert(list_end(l)   == list_iter_move(list_begin(l), 2));
    assert(list_begin(l) == list_iter_move(list_end(l), -3));
    assert(t1 == list_at(list_begin(l)));
    assert(t2 == list_at_offset(list_begin(l), 1));

    t4 = list_insert(list_end(l), make());
    t4->a = 4;
    assert(3 == list_size(l));
    assert(list_begin(l) != list_end(l));
    assert(list_end(l)   == list_iter_move(list_begin(l), 3));
    assert(list_begin(l) == list_iter_move(list_end(l), -4));
    assert(t1 == list_at(list_begin(l)));
    assert(t2 == list_at_offset(list_begin(l), 1));
    assert(t4 == list_at_offset(list_begin(l), 2));

    t3 = list_insert(list_prev(list_end(l)), make());
    t3->a = 3;
    assert(4 == list_size(l));
    assert(t1 == list_at(list_begin(l)));
    assert(t2 == list_at_offset(list_begin(l), 1));
    assert(t3 == list_at_offset(list_begin(l), 2));
    assert(t4 == list_at_offset(list_begin(l), 3));

    free(list_unlink(list_next(list_begin(l))));
    t2 = NULL;

    assert(3 == list_size(l));
    assert(t1 == list_at(list_begin(l)));
    assert(t3 == list_at_offset(list_begin(l), 1));
    assert(t4 == list_at_offset(list_begin(l), 2));

    list_erase_range(list_begin(l), list_next(list_next(list_begin(l))), free);
    assert(1 == list_size(l));
    assert(t4 == list_at(list_begin(l)));
    assert(list_begin(l) != list_end(l));
    assert(list_end(l)   == list_next(list_begin(l)));
    assert(list_begin(l) == list_prev(list_prev(list_end(l))));

    list_delete(&l);
    assert(NULL == l);

    l = list_new(offsetof(struct test, link));
    t1 = list_insert(list_end(l), make());
    t1->a = 1;
    t1 = list_insert(list_end(l), make());
    t1->a = 2;
    t1 = list_insert(list_end(l), make());
    t1->a = 3;
    assert(((struct test *)list_at(list_iter_move(list_begin(l), 0)))->a == 1);
    assert(((struct test *)list_at(list_iter_move(list_begin(l), 1)))->a == 2);
    assert(((struct test *)list_at(list_iter_move(list_begin(l), 2)))->a == 3);

    list_splice(list_begin(l), list_prev(list_end(l)));
    assert(((struct test *)list_at(list_iter_move(list_begin(l), 0)))->a == 3);
    assert(((struct test *)list_at(list_iter_move(list_begin(l), 1)))->a == 1);
    assert(((struct test *)list_at(list_iter_move(list_begin(l), 2)))->a == 2);

    list_splice(list_end(l), list_begin(l));
    assert(((struct test *)list_at(list_iter_move(list_begin(l), 0)))->a == 1);
    assert(((struct test *)list_at(list_iter_move(list_begin(l), 1)))->a == 2);
    assert(((struct test *)list_at(list_iter_move(list_begin(l), 2)))->a == 3);

    list_splice(list_begin(l), list_begin(l));
    assert(((struct test *)list_at(list_iter_move(list_begin(l), 0)))->a == 1);
    assert(((struct test *)list_at(list_iter_move(list_begin(l), 1)))->a == 2);
    assert(((struct test *)list_at(list_iter_move(list_begin(l), 2)))->a == 3);

    list_delete(&l);
    assert(NULL == l);

    return 0;
}
