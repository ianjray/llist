# llist
Doubly-Linked List Implementation in C

The API borrows from C++ std::list, but embeds `LIST_NODE` for flexibility.

## Example

```c
#include <assert.h>
#include <liblist/llist.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

struct node
{
    int a;
    LIST_NODE(link);
};

static struct node *make(int a)
{
    struct node * node = calloc(1, sizeof(struct node));
    node->a = a;
    return node;
}

int main(void)
{
    struct list *l;
    struct list_iter *it;
    struct node *n;

    l = list_new(offsetof(struct node, link));

    // Initially empty.
    assert(list_empty(l));
    assert(list_size(l) == 0);

    // list_begin references the first element.
    // list_end references a placeholder element one-past the end of the list.
    assert(list_begin(l) == list_end(l));

    // Insert at end is equivalent to append.
    list_insert(list_end(l), make(3));

    assert(!list_empty(l));
    assert(list_size(l) == 1);
    assert(list_begin(l) != list_end(l));

    // Insert at beginning is equivalent to prepend.
    it = list_insert(list_begin(l), make(2));

    // Insert returns an iterator to the newly inserted item.
    it = list_insert(it, make(1));
    assert(list_size(l) == 3);

    // Iterators may be used to examine list elements -- O(n) time.
    assert(((struct node *)list_at(list_advance(list_begin(l), 0)))->a == 1);
    assert(((struct node *)list_at(list_advance(list_begin(l), 1)))->a == 2);
    assert(((struct node *)list_at(list_advance(list_begin(l), 2)))->a == 3);

    list_insert(list_begin(l), make(0));

    list_insert(list_end(l), make(0));
    ((struct node *)list_at(list_advance(list_end(l), -1)))->a = 4;
    assert(list_size(l) == 5);

    list_erase(it, free);
    assert(list_size(l) == 4);

    n = list_at(list_advance(list_begin(l), 1));
    assert(n->a == 2);

    assert(((struct node *)list_at(list_advance(list_begin(l), 0)))->a == 0);
    assert(((struct node *)list_at(list_advance(list_begin(l), 1)))->a == 2);
    assert(((struct node *)list_at(list_advance(list_begin(l), 2)))->a == 3);
    assert(((struct node *)list_at(list_advance(list_begin(l), 3)))->a == 4);

    list_push_back(l, make(2));

    // Forward iteration.
    // Outputs: 0, 2, 3, 4, 2.
    for (it = list_begin(l); it != list_end(l); it = list_next(it)) {
        n = list_at(it);
        printf("%d\n", n->a);
        n->a *= 10;
    }

    // Mutation during iteration.
    for (it = list_begin(l); it != list_end(l); ) {
        struct list_iter *next = list_next(it);
        n = list_at(it);
        if (n->a == 20) {
            list_erase(it, free);
        }
        it = next;
    }

    free(list_pop_front(l));

    // Use const iterators for read-only access.
    // Outputs: 30, 40.
    for (const struct list_iter *cit = list_cbegin(l); cit != list_cend(l); cit = list_cnext(cit)) {
        const struct node *cn = list_at_const(cit);
        printf("%d\n", cn->a);
    }

    list_delete(l, free);

    return 0;
}
```

## Installation

```bash
./configure
make
sudo make install
```

## Requirements

- C99 or later
- POSIX-compatible system (for `sys/types.h`)

## Thread Safety

This library is **not** thread-safe.
