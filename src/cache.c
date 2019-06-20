#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hashtable.h"
#include "cache.h"

/**
 * Allocate a cache entry
 */
struct cache_entry *alloc_entry(char *path, char *content_type, void *content, int content_length)
{
    ///////////////////
    // IMPLEMENT ME! //
    ///////////////////
    struct cache_entry *new_cache_entry = malloc(sizeof(struct cache_entry));

    // copy path
    new_cache_entry->path = malloc(strlen(path) + 1);
    strcpy(new_cache_entry->path, path);

    //copy content_type
    new_cache_entry->content_type = malloc(strlen(content_type) + 1);
    strcpy(new_cache_entry->content_type, content_type);

    //copy content
    new_cache_entry->content = malloc(content_length);
    memcpy(new_cache_entry->content, content, content_length);

    //copy length
    new_cache_entry->content_length = content_length;

    return new_cache_entry;
}

/**
 * Deallocate a cache entry
 */
void free_entry(struct cache_entry *entry)
{
    ///////////////////
    // IMPLEMENT ME! //
    ///////////////////
    free(entry->path);
    free(entry->content_type);
    free(entry->content);
    free(entry);
}

/**
 * Insert a cache entry at the head of the linked list
 */
void dllist_insert_head(struct cache *cache, struct cache_entry *ce)
{
    // Insert at the head of the list
    if (cache->head == NULL)
    {
        cache->head = cache->tail = ce;
        ce->prev = ce->next = NULL;
    }
    else
    {
        cache->head->prev = ce;
        ce->next = cache->head;
        ce->prev = NULL;
        cache->head = ce;
    }
}

/**
 * Move a cache entry to the head of the list
 */
void dllist_move_to_head(struct cache *cache, struct cache_entry *ce)
{
    if (ce != cache->head)
    {
        if (ce == cache->tail)
        {
            // We're the tail
            cache->tail = ce->prev;
            cache->tail->next = NULL;
        }
        else
        {
            // We're neither the head nor the tail
            ce->prev->next = ce->next;
            ce->next->prev = ce->prev;
        }

        ce->next = cache->head;
        cache->head->prev = ce;
        ce->prev = NULL;
        cache->head = ce;
    }
}

/**
 * Removes the tail from the list and returns it
 * 
 * NOTE: does not deallocate the tail
 */
struct cache_entry *dllist_remove_tail(struct cache *cache)
{
    struct cache_entry *oldtail = cache->tail;

    cache->tail = oldtail->prev;
    cache->tail->next = NULL;

    cache->cur_size--;

    return oldtail;
}

/**
 * Create a new cache
 * 
 * max_size: maximum number of entries in the cache
 * hashsize: hashtable size (0 for default)
 */
struct cache *cache_create(int max_size, int hashsize)
{
    ///////////////////
    // IMPLEMENT ME! //
    ///////////////////
    struct cache *new_cache = malloc(sizeof(struct cache));

    new_cache->index = hashtable_create(hashsize, NULL);
    new_cache->head = NULL;
    new_cache->tail = NULL;
    new_cache->max_size = max_size;
    new_cache->cur_size = 0;

    return new_cache;
}

void cache_free(struct cache *cache)
{
    struct cache_entry *cur_entry = cache->head;

    hashtable_destroy(cache->index);

    while (cur_entry != NULL)
    {
        struct cache_entry *next_entry = cur_entry->next;

        free_entry(cur_entry);

        cur_entry = next_entry;
    }

    free(cache);
}

/**
 * Store an entry in the cache
 *
 * This will also remove the least-recently-used items as necessary.
 * 
 * NOTE: doesn't check for duplicate cache entries
 */
void cache_put(struct cache *cache, char *path, char *content_type, void *content, int content_length)
{
    ///////////////////
    // IMPLEMENT ME! //
    ///////////////////
    // Allocate a new cache entry with the passed parameters.
    struct cache_entry *new_entry = alloc_entry(path, content_type, content, content_length);

    // Insert the entry at the head of the doubly-linked list.
    dllist_insert_head(cache, new_entry);

    // Store the entry in the hashtable as well, indexed by the entry's path.
    hashtable_put(cache->index, path, new_entry);

    // Increment the current size of the cache.
    cache->cur_size++;

    // If the cache size is greater than the max size:
    if (cache->cur_size > cache->max_size)
    {
        // Remove the cache entry at the tail of the linked list.
        struct cache_entry *last_tail = dllist_remove_tail(cache);

        // Remove that same entry from the hashtable, using the entry's path and the hashtable_delete function.
        hashtable_delete(cache->index, last_tail->path);

        // Free the cache entry.
        free_entry(last_tail);

        // Ensure the size counter for the number of entries in the cache is correct.
        if (cache->cur_size > cache->max_size)
        {
            cache->cur_size--;
        }
    }
}

/**
 * Retrieve an entry from the cache
 */
struct cache_entry *cache_get(struct cache *cache, char *path)
{
    ///////////////////
    // IMPLEMENT ME! //
    ///////////////////

    // Attempt to find the cache entry pointer by path in the hash table.
    void *entry = hashtable_get(cache->index, path);

    // If not found, return NULL.
    if (entry == NULL)
    {
        return NULL;
    }
    else
    {
        // Move the cache entry to the head of the doubly-linked list.
        dllist_move_to_head(cache, entry);
        // Return the cache entry pointer.
        return cache->head;
    }
}
