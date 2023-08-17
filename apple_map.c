#include "apple_map.h"

#include <stdlib.h>
#include <string.h>

typedef struct bucket bucket;

/**
 * @brief      Hashmap is a data structure, that maps keys to values. Values in the
 *             apple map implementation are pointer values or integral types.
 *
 * @version    0.1.0
 */
struct apple_map
{
  bucket *buckets;

  bucket *first, *last;

  size_t capacity;
  size_t len;
  size_t tombstone_len;
};

typedef struct bucket
{
  bucket *next;

  const void *key;
  size_t key_size;
  uint32_t hash;
  uintptr_t value;
} bucket;

static bucket *resolve(apple_map *map, const void *key, size_t key_size, uint32_t hash);

static inline uint32_t fnv_1a_hash(const unsigned char *data, size_t size);

static bucket *resize_entry(apple_map *map, bucket *entry);

const size_t DEFAULT_CAPACITY = 30;

/**
 * @brief      Creates a new empty hashmap.
 * @returns    A newly allocated empty hashmap.
 *
 * @version    0.1.0
 */
apple_map *apple_map_new(void)
{
  apple_map *map = malloc(sizeof(apple_map));

  if (map == NULL)
  {
    return NULL;
  }

  map->buckets = calloc(DEFAULT_CAPACITY, sizeof(bucket));
  map->first = NULL;
  map->last = (bucket *)&map->first;

  map->capacity = DEFAULT_CAPACITY;
  map->len = 0;
  map->tombstone_len = 0;

  return map;
}

/**
 * @brief              Frees the hashmap object and inner buckets.
 * @details            This function doesn't free the key-value pairs. You can achieve that by
 *                     using `apple_map_iter` with callback freeing the hashmap entries.
 * @param map          The hashmap object to free.
 *
 * @version            0.1.0
 */
inline void apple_map_free(apple_map *map)
{
  free(map->buckets);
  free(map);
}

/**
 * @brief              Resolves a key-value pair from the hashmap.
 *
 * @param map          The hashmap, from which the key-value pair will be resolved.
 * @param key          The key to resolve.
 * @param key_size     The size of the key.
 * @param out_value    The reference to a value to store the resolved value.
 *
 * @returns            The resolved value.
 *
 * @version            0.1.0
 */
bool apple_map_get(apple_map *map, const void *key, size_t key_size, uintptr_t *out_value)
{
  uint32_t hash = fnv_1a_hash(key, key_size);
  bucket *entry = resolve(map, key, key_size, hash);

  *out_value = entry->value;

  return entry->key != NULL;
}

static bucket *resolve(apple_map *map, const void *key, size_t key_size, uint32_t hash)
{
  uint32_t index = hash % map->capacity;

  while (true)
  {
    bucket *entry = &map->buckets[index];

    bool null_key = entry->key == NULL;
    bool null_value = entry->value == 0;

    if ((null_key && null_value) ||
        (!null_key &&
         entry->key_size == key_size &&
         entry->hash == hash &&
         memcmp(entry->key, key, key_size) == 0))
    {
      return entry;
    }

    index = (index + 1) % map->capacity;
  }
}

static inline uint32_t fnv_1a_hash(const unsigned char *data, size_t size)
{
  size_t blocks_count = size / 8;
  uint64_t hash = 2166136261u;

  for (size_t i = 0; i < blocks_count; i++)
  {
    hash ^= (uint64_t)data[0] << 0 | (uint64_t)data[1] << 8 |
            (uint64_t)data[2] << 16 | (uint64_t)data[3] << 24 |
            (uint64_t)data[4] << 32 | (uint64_t)data[5] << 40 |
            (uint64_t)data[6] << 48 | (uint64_t)data[7] << 56;
    hash *= 0xbf58476d1ce4e5b9;
    data += 8;
  }

  uint64_t last = size & 0xff;
  switch (size % 8)
  {
  case 7:
    last |= (uint64_t)data[6] << 56;
  case 6:
    last |= (uint64_t)data[5] << 48;
  case 5:
    last |= (uint64_t)data[4] << 40;
  case 4:
    last |= (uint64_t)data[3] << 32;
  case 3:
    last |= (uint64_t)data[2] << 24;
  case 2:
    last |= (uint64_t)data[1] << 16;
  case 1:
    last |= (uint64_t)data[0] << 8;
    hash ^= last;
    hash *= 0xd6e8feb86659fd93;
  }

  return (uint32_t)(hash ^ hash >> 32);
}

const float MAX_CAPACITY_PERCENTAGE = 0.75;

/**
 * @brief            Inserts a key-value pair into the hashmap.
 * @details          Function doesn't copy a key, so you should guarantee its lifetime.
 *
 * @param map        The hashmap, into which the key-value pair will be inserted.
 * @param key        The key, to insert into the hashmap.
 * @param key_size   The size of the key.
 * @param value      The value, to insert into the hashmap.
 *
 * @version          0.1.0
 */
void apple_map_insert(apple_map *map, const void *key, size_t key_size, uintptr_t value)
{
  if (map->len + 1 > MAX_CAPACITY_PERCENTAGE * map->capacity)
  {
    apple_map_resize(map);
  }

  uint32_t hash = fnv_1a_hash(key, key_size);
  bucket *entry = resolve(map, key, key_size, hash);

  if (entry->key == NULL)
  {
    map->last->next = entry;
    map->last = entry;

    entry->next = NULL;

    map->len++;

    entry->key = key;
    entry->key_size = key_size;

    entry->hash = hash;
  }

  entry->value = value;
}

/**
 * @brief              Tries to resolve a key-value pair from the hashmap. If it fails, it adds the
 *                     entry into the hashmap, with the value read from `out_in`. If it doesn't, `out_in`
 *                     will be set to the value from the hashmap entry successfully resolved by key.
 *
 * @param map          The hashmap, from which the key-value pair will be resolved.
 * @param key          The key to resolve.
 * @param key_size     The size of the key.
 * @param out_in       The reference to a value to store it in a new hashmap entry or the
 *                     value that will be set to the value of successfully resolved entry.
 *
 * @returns            `true` if the key-value pair already exists.
 *                     `false` otherwise.
 *
 * @version            0.1.0
 */
bool apple_map_get_or_insert(apple_map *map, const void *key, size_t key_size, uintptr_t *out_in)
{
  if (map->len + 1 > MAX_CAPACITY_PERCENTAGE * map->capacity)
  {
    apple_map_resize(map);
  }

  uint32_t hash = fnv_1a_hash(key, key_size);
  bucket *entry = resolve(map, key, key_size, hash);

  if (entry->key == NULL)
  {
    map->last->next = entry;
    map->last = entry;
    entry->next = NULL;

    map->len++;

    entry->value = *out_in;
    entry->key = key;
    entry->key_size = key_size;
    entry->hash = hash;

    return false;
  }

  *out_in = entry->value;

  return true;
}

/**
 * @brief            Removes a key-value pair resolved by key from the hashmap.
 * @param map        The hashmap, from which the key-value pair will be removed.
 * @param  key       The key, via which entry is resolved.
 * @param  key_size  The size of the key.
 *
 * @version          0.1.0
 */
void apple_map_remove(apple_map *map, const void *key, size_t key_size)
{
  uint32_t hash = fnv_1a_hash(key, key_size);
  bucket *entry = resolve(map, key, key_size, hash);

  if (entry->key != NULL)
  {
    entry->key = NULL;
    entry->value = 0xDEAD;

    map->tombstone_len++;
  }
}

/**
 * @brief              Removes a key-value pair resolved by key from the hashmap. But unlike
 *                     `apple_map_remove`, it will free the key-value pair's data via the `callback`.
 * @param map          The hashmap, from which the key-value pair will be removed.
 * @param  key         The key, via which entry is resolved.
 * @param  key_size    The size of the key.
 * @param  callback    The callback, that will be called to free the key-value pair.
 * @param  user        User pointer is a pointer that you can use in the `callback`.
 *
 * @version            0.1.0
 */
void apple_map_remove_free(apple_map *map, const void *key, size_t key_size,
                           apple_map_callback callback, void *user)
{
  uint32_t hash = fnv_1a_hash(key, key_size);
  bucket *entry = resolve(map, key, key_size, hash);

  if (entry->key != NULL)
  {
    callback((void *)entry->key, entry->key_size, entry->value, user);

    entry->key = NULL;
    entry->value = 0xDEAD;

    map->tombstone_len++;
  }
}

/**
 * @brief              Similiar to `apple_map_insert`, but when trying to overwrite a hashmap entry,
 *                     it will free the old entry's data via callback using `callback`.
 * @param map          The hashmap, into which the key-value pair will be soft-inserted.
 * @param key          The key, to insert into the hashmap.
 * @param key_size     The size of the key.
 * @param value        The value, to insert into the hashmap.
 * @param callback     The callback, that will be called when the key-value pair already exists.
 * @param user         User pointer is a pointer that you can use in the `callback`.
 *
 * @version            0.1.0
 */
void apple_map_soft_insert(apple_map *map, const void *key, size_t key_size,
                           uintptr_t value, apple_map_callback callback, void *user)
{
  if (map->len + 1 > MAX_CAPACITY_PERCENTAGE * map->capacity)
  {
    apple_map_resize(map);
  }

  uint32_t hash = fnv_1a_hash(key, key_size);
  bucket *entry = resolve(map, key, key_size, hash);

  if (entry->key == NULL)
  {
    map->last->next = entry;
    map->last = entry;
    entry->next = NULL;

    map->len++;

    entry->key = key;
    entry->key_size = key_size;
    entry->value = value;
    entry->hash = hash;

    return;
  }

  callback((void *)entry->key, key_size, entry->value, user);

  entry->key = key;
  entry->value = value;
}

/**
 * @brief            Returns the number of entries in the hashmap.
 * @returns          The number of entries in the hashmap.
 *
 * @version          0.1.0
 */
inline size_t apple_map_len(apple_map *map)
{
  return map->len - map->tombstone_len;
}

const float RESIZE_FACTOR_PERCENTAGE = 2;

/**
 * @brief              Resizes the hashmap to a new capacity when the hashmap is full.
 * @param map          The hashmap to iterate.
 *
 * @version            0.1.0
 */
void apple_map_resize(apple_map *map)
{
  bucket *old_buckets = map->buckets;

  map->capacity *= RESIZE_FACTOR_PERCENTAGE;
  map->buckets = calloc(map->capacity, sizeof(bucket));

  map->last = (bucket *)&map->first;

  map->len -= map->tombstone_len;
  map->tombstone_len = 0;

  do
  {
    bucket *current = map->last->next;

    if (current->key == NULL)
    {
      map->last->next = current->next;
      continue;
    }

    map->last->next = resize_entry(map, map->last->next);
    map->last = map->last->next;
  } while (map->last->next != NULL);

  free(old_buckets);
}

static bucket *resize_entry(apple_map *map, bucket *entry)
{
  uint32_t idx = entry->hash % map->capacity;

  while (true)
  {
    bucket *new_entry = &map->buckets[idx];

    if (entry->key == NULL)
    {
      *new_entry = *entry;
      return new_entry;
    }

    idx = (idx + 1) % map->capacity;
  }
}

/**
 * @brief              Iterates through the hashmap, using the `callback`.
 * @param map          The hashmap to iterate.
 * @param callback     The callback, that will be called on each entry.
 * @param user         User pointer is a pointer that you can use in the `callback`.
 *
 * @version            0.2.0
 */
void apple_map_iter(apple_map *map, apple_map_callback callback, void *user)
{
  bucket *current = map->first;

  while (current != NULL)
  {
    if (current->key != NULL)
      callback((void *)current->key, current->key_size, current->value, user);

    current = current->next;
  }
}