# llist
Doubly-Linked List Implementation in C

The API borrows from C++ std::list, but exposes `struct list_node` for flexibility.

## Example

```c
#include <assert.h>
#include <liblist/llist.h>
#include <stdio.h>
#include <stdlib.h>

struct node
{
    int a;
    struct list_node link;
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
    struct node *n;

    l = list_new(offsetof(struct node, link));

    // Initially empty.
    assert(list_size(l) == 0);

    // list_begin references the first element.
    // list_end references a placeholder element one-past the end of the list.
    assert(list_begin(l) == list_end(l));

    // Elements may be appended.
    // Insert before list_end.
    ((struct node *)list_insert(list_end(l), make(0)))->a = 2;
    list_insert(list_end(l), make(3));
    assert(list_size(l) == 2);

    // Or prepended.
    // Insert before the first element (list_begin).
    n = list_insert(list_begin(l), make(1));
    assert(list_size(l) == 3);

    // Iterators may be used to examine list elements.
    assert(((struct node *)list_at_offset(list_begin(l), 0))->a == 1);
    assert(((struct node *)list_at_offset(list_begin(l), 1))->a == 2);
    assert(((struct node *)list_at_offset(list_begin(l), 2))->a == 3);

    list_insert(list_begin(l), make(0));

    list_insert(list_end(l), make(0));
    ((struct node *)list_at_offset(list_end(l), -1))->a = 4;
    assert(list_size(l) == 5);

    // An element may be unlinked (removed) from the list.
    free(list_unlink((struct list_iter *)(&n->link)));
    assert(list_size(l) == 4);

    n = list_at(list_begin(l));
    n = list_at_offset((struct list_iter *)(&n->link), 1);
    assert(n->a == 2);

    assert(((struct node *)list_at_offset(list_begin(l), 0))->a == 0);
    assert(((struct node *)list_at_offset(list_begin(l), 1))->a == 2);
    assert(((struct node *)list_at_offset(list_begin(l), 2))->a == 3);
    assert(((struct node *)list_at_offset(list_begin(l), 3))->a == 4);

    list_erase_all(l, free);

    assert(list_size(l) == 0);

    list_delete(&l);

    return 0;
}
```
