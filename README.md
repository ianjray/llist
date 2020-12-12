# llist
Doubly-Linked List Implementation in C

The API is modelled on that of C++ std::list, as far as this is possible in C.

## Example

```c
#include <assert.h>
#include <liblist/llist.h>

struct node 
{   
    int a;
};      
            
int main(void)
{
    struct list *l;
    struct list_iter *it;

    // Constructor takes size of list element, and optional destructor.
    l = list_new(sizeof(struct node), NULL);

    assert(list_empty(l));
    assert(list_size(l) == 0);
    assert(list_begin(l) == list_end(l));

    // Elements may be appended.
    ((struct node *)list_push_back(l))->a = 2;
    // Or prepended.
    ((struct node *)list_push_front(l))->a = 1;
    ((struct node *)list_push_back(l))->a = 3;

    // Accessors are provided for the front (first) and back (last) elements.
    assert(((struct node *)list_front(l))->a == 1);
    assert(((struct node *)list_back(l))->a == 3);

    // Iterators may be used to moved through the list elements.
    it = list_begin(l);
    assert(((struct node *)list_iter_deref(it))->a == 1);
    it = list_iter_inc(it);
    assert(((struct node *)list_iter_deref(it))->a == 2);
    it = list_iter_inc(it);
    assert(((struct node *)list_iter_deref(it))->a == 3);
    assert(list_iter_inc(it) == list_end(l));

    assert(list_front(l) == list_iter_deref(list_begin(l)));
    assert(list_back(l) == list_iter_deref(list_iter_dec(list_end(l))));

    assert(((struct node *)list_iter_deref(list_iter_advance(list_begin(l), 0)))->a == 1);
    assert(((struct node *)list_iter_deref(list_iter_advance(list_begin(l), 1)))->a == 2);
    assert(((struct node *)list_iter_deref(list_iter_advance(list_begin(l), 2)))->a == 3);

    // A new element may be inserted before an iterator.
    ((struct node *)list_insert(l, list_begin(l)))->a = 0;
    ((struct node *)list_insert(l, list_end(l)))->a = 4;

    // An element may be erased.
    list_erase(l, list_iter_inc(list_begin(l)));

    assert(((struct node *)list_iter_deref(list_iter_advance(list_begin(l), 0)))->a == 0);
    assert(((struct node *)list_iter_deref(list_iter_advance(list_begin(l), 1)))->a == 2);
    assert(((struct node *)list_iter_deref(list_iter_advance(list_begin(l), 2)))->a == 3);
    assert(((struct node *)list_iter_deref(list_iter_advance(list_begin(l), 3)))->a == 4);

    // An element may be moved.
    list_splice(l, list_end(l), list_begin(l));

    assert(((struct node *)list_iter_deref(list_iter_advance(list_begin(l), 0)))->a == 2);
    assert(((struct node *)list_iter_deref(list_iter_advance(list_begin(l), 1)))->a == 3);
    assert(((struct node *)list_iter_deref(list_iter_advance(list_begin(l), 2)))->a == 4);
    assert(((struct node *)list_iter_deref(list_iter_advance(list_begin(l), 3)))->a == 0);

    list_delete(l);

    return 0;
}
```
