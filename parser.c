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
	int i;
	json_t *elem;

	for (i=0; i < mapsz; i++) {
		switch(map[i].type) {
			case JSON_STRING:
				elem = json_string(map[i].val);
				break;
			case JSON_INTEGER:
				elem = json_integer(*((int*)map[i].val));
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

char * compile_json_string(struct json_map *map, int mapsz)
{
	char *result;
	json_t *obj;

	obj = json_object();

	fill_json_objects(obj, map, mapsz);	

	result = json_dumps(obj, JSON_COMPACT);
	json_decref(obj);
	return result;
}


