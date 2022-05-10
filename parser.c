#include <string.h>
#include <errno.h>
#include <jansson.h>
#include <parser.h>


int parser_set_val(struct json_map *map, int mapsz, char *key, void *val)
{
	int i;

	for (i=0; i < mapsz; i++) {
		if (!strcmp(map[i].key, key)) {
			if(!strcmp(map[i].val, TBD))
				map[i].flags |= MAP_FLAG_WAS_TBD;
			map[i].val = val;
			return 0;
		}
	}
	return -ENOENT;
}

void parser_set_flags(struct json_map *map, int mapsz, char *key, int flags)
{
	int i;

	for (i=0; i < mapsz; i++) {
		if (!strcmp(map[i].key, key)) {
			map[i].flags |= flags;
			return;
		}
	}
	return;
}

int parser_reset_tmpl(struct json_map *map, int mapsz)
{
	int i;
	for(i=0; i < mapsz; i++) {
		if (map[i].num_children)
			parser_reset_tmpl(map[i].children, map[i].num_children);
		if (map[i].flags & MAP_FLAG_WAS_TBD)
			map[i].val = TBD;
		map[i].flags = 0; 
	}
	return 0;
}

static void fill_json_objects(json_t *obj, struct json_map *map, int mapsz)
{
	size_t i, j, k;
	struct json_array_map *array_map;
	void *array_data;
	json_t *array_elem_obj;
	json_t *elem;

	for (i=0; i < (size_t)mapsz; i++) {
		switch(map[i].type) {
			case JSON_STRING:
				elem = json_string(map[i].val);
				break;
			case JSON_INTEGER:
				elem = json_integer(*((int*)map[i].val));
				break;
			case JSON_ARRAY:
				elem = json_array();
				array_map = map[i].val;
				for (j=0;j<array_map->num_elem;j++) {
					array_data = array_map->storage + (array_map->storage_elem_size * j);
					for(k=0;k<array_map->elem_size;k++) {
						array_map->map[k].val = array_data + k;
					}
					array_elem_obj = json_object();
					fill_json_objects(array_elem_obj, array_map->map, array_map->elem_size);
					json_array_set(elem, j, array_elem_obj);
				}
				break;
			case JSON_OBJECT:
				elem = json_object();
				if (!(map[i].flags & MAP_FLAG_EMPTY_OBJ))
					fill_json_objects(elem, map[i].children, map[i].num_children);
				break;
			default:
				fprintf(stderr, "Unknown json type %d\n", map[i].type);
				break;
		}
		json_object_set(obj, map[i].key, elem);
		json_decref(elem);
	}
}

struct json_array_map *parser_alloc_array(struct json_map *map, size_t num_elems_per_entry, size_t storage_size)
{
	struct json_array_map *new;
	new = calloc(sizeof(struct json_array_map),1);
	new->elem_size = num_elems_per_entry;
	new->storage_elem_size = storage_size;
	new->map = map;
	return new;
}

void *parser_next_element(struct json_array_map *map, int *index)
{
	*index = map->num_elem;
	map->num_elem++;
	map = realloc(map, sizeof(struct json_array_map) + (sizeof(void*) * map->elem_size * map->num_elem));
	map->storage = realloc(map->storage, map->storage_elem_size * map->num_elem);
	return map->storage + (*index * map->storage_elem_size);
}

int parser_set_array_val(struct json_array_map *map, int index, struct json_map *parser, int mapsz,  char *key, void *addr)
{
	int parser_idx;

	for (parser_idx=0; parser_idx < mapsz; parser_idx++) {
		if (!strcmp(parser[parser_idx].key, key)) {
			map->val[(index * map->num_elem) + parser_idx] = addr;
			return 0;
		}
	}
	return -1;
}

void parser_free_array(struct json_array_map *map)
{
	free(map->storage);
	free(map);
}

char * compile_json_string(struct json_map *map, int mapsz)
{
	char *result;
	json_t *obj;

	obj = json_object();

	fill_json_objects(obj, map, mapsz);	

	result = json_dumps(obj, JSON_COMPACT|JSON_ENCODE_ANY);
	json_decref(obj);
	return result;
}


