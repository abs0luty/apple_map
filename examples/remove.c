#include "../apple_map.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int main()
{
  apple_map *map = apple_map_new();

  char *hello = malloc(5);
  strcpy(hello, "hello");

  apple_map_insert(map, hello, sizeof(hello) - 1, 1);

  printf("map.len() = %ld\n", apple_map_len(map));

  apple_map_remove(map, hello, sizeof(hello) - 1);

  printf("map.len() = %ld\n", apple_map_len(map));

  free(hello);
  apple_map_free(map);
}
