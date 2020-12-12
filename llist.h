#ifndef LLIST__H
#define LLIST__H

#include <stddef.h>
#include <sys/types.h>

#ifdef __has_attribute
# define PUBLIC __attribute__ ((visibility("default")))
#else
# define PUBLIC /*NOTHING*/
#endif

/// Doubly-linked list fields.
/// Must be embedded in each list element, at offset defined in @c list_new.
struct list_node {
    /// Next node.
    struct list_node *next;
    /// Previous node.
    struct list_node *prev;
    /// Back-pointer to list.
    struct list *list;
};

/// List object.
struct list;

/// Iterator object.
struct list_iter;

/// Constructor.
/// @param offset The offset to @c list_node in list elements.
/// @return Reference to list (to be freed with @c free).
struct list *list_new(size_t offset) PUBLIC;

/// Destructor.
void list_delete(struct list **) PUBLIC;

/// @return The number of elements in the list.
size_t list_size(const struct list *) PUBLIC;

/// @return Iterator to the beginning.
struct list_iter *list_begin(struct list *) PUBLIC;

/// @return Iterator to the end.
struct list_iter *list_end(struct list *) PUBLIC;

/// Insert @c element before iterator.
/// @return Reference to the inserted element.
void *list_insert(struct list_iter *, void *element) PUBLIC;

/// Insert @c element at front of @c list.
/// @return Reference to the inserted element.
#define list_push_front(list, element) list_insert(list_begin(list), element)

/// Insert @c element at end of @c list.
/// @return Reference to the inserted element.
#define list_push_back(list, element) list_insert(list_end(list), element)

/// Remove element at iterator from the list.
/// @return Reference to removed element.
void *list_unlink(struct list_iter *) PUBLIC;

/// Remove range of elements [begin,end) and call @c destructor for each removed element.
void list_erase_range(struct list_iter *begin, struct list_iter *end, void (*destructor)(void *)) PUBLIC;

/// Remove all elements from list @c list, calling @c destructor for each.
#define list_erase_all(list, destructor) list_erase_range(list_begin(list), list_end(list), destructor)

/// @return Iterator to next element (bounded by @c list_end).
#define list_next(it) list_iter_move(it, 1)

/// @return Iterator to previous element (bounded by @c list_begin).
#define list_prev(it) list_iter_move(it, -1)

/// @return Iterator moved by @c offset (within bounds @c list_begin to @c list_end).
struct list_iter *list_iter_move(struct list_iter *, ssize_t offset) PUBLIC;

/// @return Reference to element for given @c iter.
void *list_at(struct list_iter *) PUBLIC;

/// @return Reference to element for given @c iter plus @c offset.
#define list_at_offset(it, offset) list_at(list_iter_move(it, offset))

/// Move element @c source_iter to before @c iter.
/// @note Both iterators must belong to the same list instance.
void list_splice(struct list_iter *iter, struct list_iter *source_iter) PUBLIC;

#endif
