#ifndef LLIST__H
#define LLIST__H

#include <stdbool.h>
#include <stddef.h>
#include <sys/types.h>


/// @brief Configure allocator interface.
/// @discussion Defaults to stdlib if not set.
/// @param alloc Allocator function, see malloc(3).
/// @param dealloc Deallocator function, see free(3).
void list_allocator_set(
        void *(*alloc)(size_t),
        void (*dealloc)(void *));

/// @brief Constructor.
/// @param element_size Element size.
/// @param element_destructor Optional destructor for elements, see free(3).
struct list *list_new(
        size_t element_size,
        void (*element_destructor)(void *));

/// @brief Destructor.
void list_delete(struct list *l);

/// @brief Get first element.
/// @return void* Pointer to element.
void *list_front(struct list *l);

/// @brief Get last element.
/// @return void* Pointer to element.
void *list_back(struct list *l);

struct list_iter;

/// @return Iterator Returns an iterator to the beginning.
struct list_iter *list_begin(struct list *l);

/// @return Iterator Returns an iterator to the end.
struct list_iter *list_end(struct list *l);

/// @brief Increment iterator position.
/// @return Iterator
struct list_iter *list_iter_inc(struct list_iter *iter);

/// @brief Decrement iterator position.
/// @return Iterator
struct list_iter *list_iter_dec(struct list_iter *iter);

/// @brief Advance iterator by @c distance.
/// @return Iterator
struct list_iter *list_iter_advance(struct list_iter *iter, ssize_t distance);

/// @brief Dereference iterator.
/// @return void* Pointer to element at iterator position.
void *list_iter_deref(struct list_iter *iter);

/// @return bool True if the list is empty.
bool list_empty(const struct list *l);

/// @return size_t The number of elements in the list.
size_t list_size(const struct list *l);

/// @brief Clears the contents of the list.
void list_clear(struct list *l);

/// @brief Insert element before @c pos.
/// @return void* Pointer to element.
void *list_insert(struct list *l, struct list_iter *it);

/// @brief Erase element.
void list_erase(struct list *l, struct list_iter *it);

/// @brief Add new element to the end.
/// @return void* Pointer to new element.
void *list_push_back(struct list *l);

/// @brief Remove the last element.
void list_pop_back(struct list *l);

/// @brief Insert an element at the beginning.
/// @return void* Pointer to new element.
void *list_push_front(struct list *l);

/// @brief Remove the first element.
void list_pop_front(struct list *l);

#endif
