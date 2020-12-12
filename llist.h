#ifndef LIBLIST_LLIST_H_
#define LIBLIST_LLIST_H_

/// Elements inserted into a list must use @c LIST_NODE to embed list management data.
/// @note An element can embed multiple @c LIST_NODE items if it participates in multiple lists.
///
/// The offset of the LIST_NODE is given to @c list_new.
/// @note A single list_node may belong to at most one list at a time.
///
/// Example:
///
///     struct my_item {
///         int value;
///         LIST_NODE(link);
///     };
///
///     struct list *l = list_new(offsetof(my_item, link));
///
/// @note All list manipulation must be done through the API.

#define LIST_NODE(name) struct list_node name

/// Functions in the API are grouped by three return type conventions:
///
/// 1. Functions that *produce or return an object pointer* return that pointer on success, or NULL on failure with errno set to indicate the error.
///
/// 2. Functions that *perform an operation* return zero on success, or -1 on failure with errno set to indicate the failure.
///
/// 3. Other functions that *return simple values* return the value directly, and silently accept invalid arguments.

#include <stdbool.h>
#include <stddef.h>
#include <sys/types.h>

#ifdef __has_attribute
# define PUBLIC __attribute__ ((visibility("default")))
#else
# define PUBLIC /*NOTHING*/
#endif

/// List object.
///
/// Provides a container that supports constant time insertion and removal of elements from anywhere in the container.
///
/// This library is **not** thread-safe.
/// Caller must synchronize access to list objects.
/// Multiple readers are safe if no writers are active.
struct list;

struct list_node {
    struct list_node *next;
    struct list_node *prev;
    struct list *list;
};

/// Constructor.
/// Create a new list that embeds @c struct @c list_node at @c offset within client elements.
/// @param offset The offset to @c list_node in list elements.
/// @note Caller is responsible for providing a valid offset.
///       Offset must be a multiple of sizeof(void*) for proper alignment.
/// @return Pointer to list on success.
/// @return NULL on failure, and errno is set to:
///   - EINVAL: Offset invalid.
///   - ENOMEM: Insufficient memory.
/// @note Memory ownership: Caller must list_delete() the returned pointer.
struct list *list_new(size_t offset) PUBLIC;

/// Destructor.
/// @param destructor Function pointer to destructor function for each element, or NULL if no destructor is needed.
/// The @c destructor is called for each element.
/// @note Invalidates all iterators.
/// @note Memory ownership: Object takes ownership of the pointer.
void list_delete(struct list *, void (*destructor)(void *)) PUBLIC;

/// Test if list is empty.
/// @return True if list is empty or NULL, false otherwise.
bool list_empty(const struct list *) PUBLIC;

/// Get number of elements in list.
/// @return The number of elements in the list, or zero if empty or NULL.
/// @note Complexity: O(1).
size_t list_size(const struct list *) PUBLIC;

/// Erases all elements from the container.
/// The @c destructor is called for each element if it is non-NULL.
/// @return 0 on success.
/// @return -1 on failure, and errno is set to:
///   - EINVAL: List invalid.
/// @note Invalidates all iterators.
int list_clear(struct list *, void (*destructor)(void *)) PUBLIC;

/// Iterator object.
/// @note Memory ownership: Owned by the object; valid until list_clear(), list_erase() (of that specific iterator),
///       or list_delete(). NOT invalidated by insertions, splice, or erasure of other elements.
struct list_iter;

/// Get iterator to first element of list.
/// @return Pointer to iterator on success.
/// @return NULL on failure, and errno is set to:
///   - EINVAL: List invalid.
/// @note list_begin(l) == list_end(l) when the list is empty.
struct list_iter *list_begin(struct list *) PUBLIC;

/// Constant variant of @c list_begin.
/// @see list_begin.
const struct list_iter *list_cbegin(const struct list *) PUBLIC;

/// Get an iterator past the last element of list.
/// @return Pointer to iterator on success.
/// @return NULL on failure, and errno is set to:
///   - EINVAL: List invalid.
/// @note This returned iterator only acts as a sentinel.
struct list_iter *list_end(struct list *) PUBLIC;

/// Constant variant of @c list_end.
/// @see list_end.
const struct list_iter *list_cend(const struct list *) PUBLIC;

/// Forward move iterator.
/// @return Pointer to iterator on success.
/// @return NULL on failure, and errno is set to:
///   - EINVAL: Iterator invalid.
/// @note Bounded by @c list_end.
struct list_iter *list_next(struct list_iter *) PUBLIC;

/// Constant variant of @c list_next.
/// @see list_next.
const struct list_iter *list_cnext(const struct list_iter *) PUBLIC;

/// Reverse move iterator.
/// @return Pointer to iterator on success.
/// @return NULL on failure, and errno is set to:
///   - EINVAL: Iterator invalid.
/// @note Bounded by @c list_begin.
struct list_iter *list_prev(struct list_iter *) PUBLIC;

/// Constant variant of @c list_prev.
/// @see list_prev.
const struct list_iter *list_cprev(const struct list_iter *) PUBLIC;

/// Advance iterator.
/// Increment iterator by @c n elements.
/// @return Pointer to iterator on success.
/// @return NULL on failure, and errno is set to:
///   - EINVAL: Iterator invalid.
///   - ERANGE: Offset out of range.
/// @note Complexity: O(n).
struct list_iter *list_advance(struct list_iter *, ssize_t n) PUBLIC;

/// Constant variant of @c list_advance.
const struct list_iter *list_advance_const(const struct list_iter *, ssize_t n) PUBLIC;

/// Get an iterator from a element.
/// The element must be currently in the list and obtained via list_at().
/// @param element Pointer to element (as returned by list_pop, list_at).
/// @param offset The offset to list_node in the element (same as list_new).
/// @return Pointer to iterator on success.
/// @return NULL on failure, and errno is set to:
///   - EINVAL: Element invalid or offset mismatch.
/// @note Memory ownership: Iterator is valid while element remains in the list.
struct list_iter *list_element(void *element, size_t offset) PUBLIC;

/// Constant variant of @c list_element.
/// @see list_element.
const struct list_iter *list_element_const(const void *element, size_t offset) PUBLIC;

/// Insert element before iterator.
/// @return Pointer to iterator on success.
/// @return NULL on failure, and errno is set to:
///   - EINVAL: Iterator or element invalid.
///   - EOVERFLOW: List cannot grow.
/// @warning The embedded list_node must be at the offset supplied to @c list_new.
/// @warning The @c element must not be already inserted to a list.
/// @note Does not invalidate existing iterators.
/// @note Memory ownership: On success the object takes ownership of the element; the caller may safely mutate any field other than @c LINK_NODE.
struct list_iter *list_insert(struct list_iter *, void *element) PUBLIC;

/// Insert element at front of list.
/// @see list_insert.
struct list_iter *list_push_front(struct list *, void *element) PUBLIC;

/// Insert element at end of list.
/// @see list_insert.
struct list_iter *list_push_back(struct list *, void *element) PUBLIC;

/// Unlink and return the first element of the list.
/// @return Pointer to element on success.
/// @return NULL on failure, and errno is set to:
///   - EINVAL: List invalid.
///   - ENOENT: List empty.
/// @note Invalidates iterators pointing to removed nodes.
/// @note Memory ownership: On success the caller regains ownership of the element.
void *list_pop_front(struct list *) PUBLIC;

/// Unlink and return the last element of the list.
/// @see list_pop_front.
void *list_pop_back(struct list *) PUBLIC;

/// Dereference iterator.
/// @return Pointer to element on success.
/// @return NULL on failure, and errno is set to:
///   - EINVAL: Iterator invalid.
///   - ENOENT: Iterator @c list_end cannot be dereferenced.
/// @note Memory ownership: Pointer remains valid while iterator is valid.
void *list_at(struct list_iter *) PUBLIC;

/// Constant variant of @c list_at.
/// @see list_at.
const void *list_at_const(const struct list_iter *) PUBLIC;

/// Remove element from the list, calling @c destructor.
/// @return 0 on success.
/// @return -1 on failure, and errno is set to:
///   - EINVAL: Iterator invalid.
///   - ENOENT: Iterator @c list_end cannot be erased.
/// @note Invalidates iterators pointing to removed nodes.
/// @note Memory ownership: On success if @c destructor is NULL then the caller regains ownership.
int list_erase(struct list_iter *, void (*destructor)(void *)) PUBLIC;

/// Move single element @c source_iter to before @c iter.
/// The splice operation moves elements without changing size.
/// @return 0 on success.
/// @return -1 on failure, and errno is set to:
///   - EINVAL: Iterator invalid or iterators belong to different list instances.
///   - ENOENT: Iterator @c list_end cannot be moved.
/// @note Both iterators must belong to the same list instance.
/// @note Does not invalidate existing iterators.
/// @note If @c source_iter is already positioned immediately before @c iter, then no change is made and function returns successfully.
int list_splice(struct list_iter *iter, struct list_iter *source_iter) PUBLIC;

#endif
