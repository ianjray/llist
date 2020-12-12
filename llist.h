#ifndef LLIST__H
#define LLIST__H

#include <stdbool.h>
#include <stddef.h>
#include <sys/types.h>

#ifdef __has_attribute
# define PUBLIC __attribute__ ((visibility("default")))
#else
# define PUBLIC /*NOTHING*/
#endif

/// @brief Configure allocator interface.
/// @discussion Defaults to stdlib if not set.
/// @param alloc Allocator function, see malloc(3).
/// @param dealloc Deallocator function, see free(3).
void list_allocator_set(
        void *(*alloc)(size_t),
        void (*dealloc)(void *)) PUBLIC;

/// @brief Constructor.
/// @param element_size Element size.
/// @param element_destructor Optional destructor for elements, see free(3).
struct list *list_new(
        size_t element_size,
        void (*element_destructor)(void *)) PUBLIC;

/// @brief Destructor.
void list_delete(struct list *l) PUBLIC;

/// @brief Get first element.
/// @return void* Pointer to element.
void *list_front(struct list *l) PUBLIC;

/// @brief Get last element.
/// @return void* Pointer to element.
void *list_back(struct list *l) PUBLIC;

struct list_iter;

/// @return Iterator Returns an iterator to the beginning.
struct list_iter *list_begin(struct list *l) PUBLIC;

/// @return Iterator Returns an iterator to the end.
struct list_iter *list_end(struct list *l) PUBLIC;

/// @brief Increment iterator position.
/// @return Iterator
struct list_iter *list_iter_inc(struct list_iter *iter) PUBLIC;

/// @brief Decrement iterator position.
/// @return Iterator
struct list_iter *list_iter_dec(struct list_iter *iter) PUBLIC;

/// @brief Advance iterator by @c distance.
/// @return Iterator
struct list_iter *list_iter_advance(struct list_iter *iter, ssize_t distance) PUBLIC;

/// @brief Dereference iterator.
/// @return void* Pointer to element at iterator position.
void *list_iter_deref(struct list_iter *iter) PUBLIC;

/// @return bool True if the list is empty.
bool list_empty(const struct list *l) PUBLIC;

/// @return size_t The number of elements in the list.
size_t list_size(const struct list *l) PUBLIC;

/// @brief Clears the contents of the list.
void list_clear(struct list *l) PUBLIC;

/// @brief Insert element before @c iter.
/// @return void* Pointer to element.
void *list_insert(struct list *l, struct list_iter *iter) PUBLIC;

/// @brief Erase element.
void list_erase(struct list *l, struct list_iter *iter) PUBLIC;

/// @brief Add new element to the end.
/// @return void* Pointer to new element.
void *list_push_back(struct list *l) PUBLIC;

/// @brief Remove the last element.
void list_pop_back(struct list *l) PUBLIC;

/// @brief Insert an element at the beginning.
/// @return void* Pointer to new element.
void *list_push_front(struct list *l) PUBLIC;

/// @brief Remove the first element.
void list_pop_front(struct list *l) PUBLIC;

/// Move element @c source_iter to before @c iter.
/// @note Both iterators must belong to the same list instance.
void list_splice(struct list *l, struct list_iter *iter, struct list_iter *source_iter) PUBLIC;

#endif
