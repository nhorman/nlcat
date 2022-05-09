#ifndef __PARSER_H__
#define __PARSER_H__
#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>

struct json_map {
	char *key;
	int type;
	void *val;
	size_t num_children;
	struct json_map *children;	
};

#define TBD "TBD"

#define ARRAY_SIZE(arr) \
	(sizeof(arr) / sizeof((arr)[0]) \
	 + sizeof(typeof(int[1 - 2 * \
		!!__builtin_types_compatible_p(typeof(arr), \
		typeof(&arr[0]))])) * 0)

#define JSON_MAP_ENTRY(jkey, jtype, jval, jnum_children, jchildren) { \
	.key = jkey, \
	.type = jtype, \
	.val = jval, \
	.num_children = jnum_children, \
	.children = jchildren, \
}

#define JSON_COMMON_TOPLEVEL(jproto, jobj, jop, jdata) {\
	.key = "protocol",\
        .type = JSON_STRING,\
        .val = jproto,\
	.num_children = ARRAY_SIZE(jdata),\
        .children = jdata,\
}

struct json_map* clone_json_template(struct json_map *tmpl);
void free_json_template(struct json_map *clone);
#endif
