#ifndef __PARSER_H__
#define __PARSER_H__
#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>

struct json_map {
	char *key;
	int type;
	void *val;
	int flags;
	size_t num_children;
	struct json_map *children;	
};

struct json_array_map {
	struct json_map *map;
	size_t num_elem;
	size_t elem_size;
	size_t storage_elem_size;
	void *storage;
	void *val[0];
};

#define TBD "TBD"

#define MAP_FLAG_WAS_TBD 1 << 0
#define MAP_FLAG_EMPTY_OBJ 1 << 1

#define ARRAY_SIZE(arr) \
	(sizeof(arr) / sizeof((arr)[0]) \
	 + sizeof(typeof(int[1 - 2 * \
		!!__builtin_types_compatible_p(typeof(arr), \
		typeof(&arr[0]))])) * 0)

#define JSON_MAP_ENTRY(jkey, jtype, jval, jnum_children, jchildren) { \
	.key = jkey, \
	.type = jtype, \
	.val = jval, \
	.flags = 0, \
	.num_children = jnum_children, \
	.children = jchildren, \
}

#define JSON_COMMON_TOPLEVEL(jproto, jobj, jop, jdata) { \
	JSON_MAP_ENTRY("protocol", JSON_STRING, jproto, 0, NULL), \
	JSON_MAP_ENTRY("obj", JSON_STRING, jobj, 0, NULL), \
	JSON_MAP_ENTRY("op", JSON_STRING, jop, 0, NULL), \
	JSON_MAP_ENTRY("data", JSON_OBJECT, NULL, ARRAY_SIZE(jdata), jdata) \
}

int parser_set_val(struct json_map *map, int mapsz, char *key, void *val);
int parser_set_array_val(struct json_array_map *map, int index, struct json_map *parser, int mapsz,  char *key, void *addr);

void parser_set_flags(struct json_map *map, int mapsz, char *key, int flags);
int parser_reset_tmpl(struct json_map *map, int mapsz);
struct json_array_map *parser_alloc_array(struct json_map *map, size_t num_elems_per_entry, size_t storage_size);
void parser_free_array(struct json_array_map *map);
void *parser_next_element(struct json_array_map *map, int *index);

char * compile_json_string(struct json_map *map, int mapsz);
#endif
