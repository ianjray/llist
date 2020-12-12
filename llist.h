#ifndef LIBLIST_LLIST_H_
#define LIBLIST_LLIST_H_

/// Linked-list library.
///
/// This object uses *intrusive nodes* where each element type embeds a `struct list_node` at a fixed offset.
/// The offset is supplied once at list construction, @see list_new.
/// The embedded list_node field must be zero-initialized before insertion, @see list_insert.
///
/// Functions that *produce or return an object pointer* return that pointer on success, or NULL on failure.
/// Examples: list_new(), list_insert(), list_at(), list_erase().
///
/// Functions that *perform an operation* with no direct return value return an int status code: 0 on success, negative errno on failure.
/// Examples: list_delete(), list_clear(), list_splice().
///
/// Functions that *report a size or count* return size_t (always defined, even for NULL list).
/// Example: list_size().
///
/// @note This library is not thread-safe.

#include <stddef.h>
#include <sys/types.h>

#ifdef __has_attribute
# define PUBLIC __attribute__ ((visibility("default")))
#else
# define PUBLIC /*NOTHING*/
#endif

/// Doubly-linked list fields.
/// Must be embedded in each list element, at offset defined in @c list_new.
/// @note A single list_node may belong to at most one list at a time.
/// @note An element can embed multiple list_node fields if it participates in multiple lists.
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

/// Constructor.
/// Create a new list that embeds @c struct @c list_node at @c offset within client elements.
/// @param offset The offset to @c list_node in list elements.
/// @return Pointer to list, or NULL on error.
/// @note Memory ownership: Caller must list_delete() the returned pointer.
struct list *list_new(size_t offset) PUBLIC;

/// Destructor.
/// @return Zero on success, negative errno otherwise.
/// @note If the list is not empty then the elements are leaked.
/// @note Memory ownership: Takes ownership of the pointer.
int list_delete(struct list *) PUBLIC;

/// Get number of elements in list.
/// @return The number of elements in the list, or zero on error.
/// @note Complexity: O(1).
size_t list_size(const struct list *) PUBLIC;

/// Test if list is empty.
/// @return Non-zero if list is empty (or NULL), zero otherwise.
#define list_empty(list) (list_size(list) == 0)

/// Iterator object.
struct list_iter;

/// @return Iterator to the beginning, or NULL on error.
/// @note list_begin(l) == list_end(l) when the list is empty.
/// @note Memory ownership: The pointer is valid until element is erased or list deleted.
struct list_iter *list_begin(struct list *) PUBLIC;

/// @return Iterator to the end, or NULL on error.
/// @note Memory ownership: The pointer is valid until element is erased or list deleted.
struct list_iter *list_end(struct list *) PUBLIC;

/// @return Constant iterator to the beginning, or NULL on error.
/// @note Memory ownership: The pointer is valid until element is erased or list deleted.
const struct list_iter *list_cbegin(const struct list *) PUBLIC;

/// @return Constant iterator to the end, or NULL on error.
/// @note Memory ownership: The pointer is valid until element is erased or list deleted.
const struct list_iter *list_cend(const struct list *) PUBLIC;

/// @return Iterator to next element (bounded by @c list_end), or NULL on error.
/// @note Memory ownership: The pointer is valid until element is erased or list deleted.
struct list_iter *list_next(struct list_iter *) PUBLIC;

/// @return Iterator to previous element (bounded by @c list_begin), or NULL on error.
/// @note Memory ownership: The pointer is valid until element is erased or list deleted.
struct list_iter *list_prev(struct list_iter *) PUBLIC;

/// @return Constant iterator to next element (bounded by @c list_end), or NULL on error.
/// @note Memory ownership: The pointer is valid until element is erased or list deleted.
const struct list_iter *list_cnext(const struct list_iter *) PUBLIC;

/// @return Constant iterator to previous element (bounded by @c list_begin), or NULL on error.
/// @note Memory ownership: The pointer is valid until element is erased or list deleted.
const struct list_iter *list_cprev(const struct list_iter *) PUBLIC;

/// @return Iterator advanced by @c offset (within bounds @c list_begin to @c list_end), or NULL on error.
/// @note Offsets are O(n).
/// @note Memory ownership: The pointer is valid until element is erased or list deleted.
struct list_iter *list_advance(struct list_iter *, ssize_t offset) PUBLIC;

/// @return Constant iterator advanced by @c offset (within bounds @c list_cbegin to @c list_cend), or NULL on error.
/// @note Offsets are O(n).
/// @note Memory ownership: The pointer is valid until element is erased or list deleted.
const struct list_iter *list_advance_const(const struct list_iter *, ssize_t offset) PUBLIC;

/// Insert element before iterator.
/// @return Pointer to element, or NULL on error.
/// @warning The embedded list_node field must be zero-initialized before insertion, otherwise behavior is undefined.
/// @note The element must not already be linked; its embedded list_node must be at the offset supplied to list_new.
/// @note Memory ownership: Caller retains ownership of the pointer.
struct list_iter *list_insert(struct list_iter *, void *element) PUBLIC;

/// Insert element at front of list.
/// @see list_insert.
/// @return Pointer to element, or NULL on error.
/// @note Memory ownership: Caller retains ownership of the pointer.
struct list_iter *list_push_front(struct list *, void *element) PUBLIC;

/// Insert element at end of list.
/// @see list_insert.
/// @return Pointer to element, or NULL on error.
/// @note Memory ownership: Caller retains ownership of the pointer.
struct list_iter *list_push_back(struct list *, void *element) PUBLIC;

/// @return Pointer to element for given iterator, or NULL on error.
/// @note Memory ownership: The pointer is valid until element is erased or list deleted.
void *list_at(struct list_iter *) PUBLIC;

/// @return Constant pointer to element for given iterator, or NULL on error.
/// @note Memory ownership: The pointer is valid until element is erased or list deleted.
const void *list_at_const(const struct list_iter *) PUBLIC;

/// Remove element from the list, calling @c destructor.
/// @return Zero on success, negative errno otherwise.
/// @note On success the iterator is invalidated and must not be used.
int list_erase(struct list_iter *, void (*destructor)(void *)) PUBLIC;

/// Remove range of elements [begin,end) from the list, calling @c destructor for each.
/// @return Zero on success, negative errno otherwise.
int list_erase_range(struct list_iter *begin, struct list_iter *end, void (*destructor)(void *)) PUBLIC;

/// Erases all elements from the container.
/// The @c destructor is called for each element.
/// @return Zero on success, negative errno otherwise.
int list_clear(struct list *, void (*destructor)(void *)) PUBLIC;

/// Move single element @c source_iter to before @c iter.
/// The splice operation moves elements without changing size.
/// @return Zero on success, negative errno otherwise.
/// @note Both iterators must belong to the same list instance.
int list_splice(struct list_iter *iter, struct list_iter *source_iter) PUBLIC;

/// Iterate forward over elements in a list.
/// @param l Pointer to list.
/// @param it Iterator variable name (struct list_iter *).
/// @note Safe for read-only traversal.
/// @note Do not insert or erase elements during traversal; iterate manually instead.
#define list_foreach(l, it) \
    for (struct list_iter *(it) = list_begin((l)); (it) != list_end((l)); (it) = list_next((it)))

/// Iterate forward over elements in a const list.
/// @param l Pointer to const list.
/// @param it Iterator variable name (const struct list_iter *).
/// @note Safe for read-only traversal.
#define list_foreach_const(l, it) \
    for (const struct list_iter *(it) = list_cbegin((l)); (it) != list_cend((l)); (it) = list_cnext((it)))

/// Iterate over elements.
/// @param l Pointer to list.
/// @param type Type of the containing struct.
/// @param var Loop variable of type (type *).
/// @note Safe for read-only traversal.
/// @note Do not insert or erase elements during traversal; iterate manually instead.
#define list_foreach_entry(l, type, var) \
    for (struct list_iter *_it = list_begin((l)); _it != list_end((l)) && ((var) = (type *)list_at(_it), 1); _it = list_next(_it))

/// Iterate over elements (const).
/// @param l Pointer to list.
/// @param type Type of the containing struct.
/// @param var Loop variable of type (type *).
#define list_foreach_entry_const(l, type, var) \
    for (const struct list_iter *_it = list_cbegin((l)); _it != list_cend((l)) && ((var) = (const type *)list_at_const(_it), 1); _it = list_cnext(_it))

#endif
