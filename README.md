# 🍎 Apple map

Apple map is a C library _written in half an hour_, that implements a hashmap using FNV-1a hashing algorithm.

Create a hashmap in 1 line of code:

```c
apple_map *map = apple_map_new();
```

Add entries into it in 1 line of code:

```c
apple_map_insert(
  map,
  "hello", // key
  sizeof("hello") - 1,
  1 // value
);
```

Remove entries from it in 1 line of code:

```c
apple_map_remove(map, "hello", sizeof("hello") - 1);
```

Resolve values by keys in 1 function call:

```c
uintptr_t value;

if (apple_map_get(map, "hello", sizeof("hello") - 1, &value)) {
  printf("map[\"hello\"] = %d\n", value);
}
```

And finally free the map:

```c
apple_map_free(map);
```

You can take a look at examples [here](https://github.com/abs0luty/apple_map/tree/main/examples).
