#include <stdio.h>
#include "../apple_map.h"

int main()
{
	apple_map *map = apple_map_new();

	printf("map.len() = %ld\n", apple_map_len(map));

	apple_map_insert(map, "hello", sizeof("hello") - 1, 1);
	apple_map_insert(map, "world", sizeof("world") - 1, 2);

	printf("map.len() = %ld\n", apple_map_len(map));

	uintptr_t expected_value;

	if (apple_map_get(map, "hello", sizeof("hello") - 1, &expected_value))
	{
		printf("map[\"hello\"] = %ld\n", expected_value);
	}
	else
	{
		printf("map[\"hello\"] = undefined\n");
	}

	if (apple_map_get(map, "world", sizeof("world") - 1, &expected_value))
	{
		printf("map[\"world\"] = %ld\n", expected_value);
	}
	else
	{
		printf("map[\"world\"] = undefined\n");
	}

	if (apple_map_get(map, "!", sizeof("!") - 1, &expected_value))
	{
		printf("map[\"!\"] = %ld\n", expected_value);
	}
	else
	{
		printf("map[\"!\"] = undefined\n");
	}
}
