#ifndef __PARSER_H__
#define __PARSER_H__
#include <jansson.h>

#define JSON_ASSIGN_STRING(obj, key, str) do { \
	json_t *____comp; \
	____comp = json_string(str); \
	json_object_set(obj, key, ____comp); \
	json_decref(____comp); \
} while(0)

#define JSON_ASSIGN_INT(obj, key, val) do { \
	json_t *____comp; \
	____comp = json_integer(val); \
	json_object_set(obj, key, ____comp); \
	json_decref(____comp); \
} while(0)

static inline json_t *create_json_child_object(json_t *parent, const char *key)
{
	json_t *obj = json_object();
	json_object_set(parent, key, obj);
	json_decref(obj);
	return obj;
}

static inline json_t *create_json_child_array(json_t *parent, const char *key)
{
	json_t *obj = json_array();
	json_object_set(parent, key, obj);
	json_decref(obj);
	return obj;
}

static inline json_t* create_json_report(json_t *obj, const char *proto, const char *object, const char *op)
{
	json_t *data;

	JSON_ASSIGN_STRING(obj, "proto", proto);
	JSON_ASSIGN_STRING(obj, "obj", object);
	JSON_ASSIGN_STRING(obj, "op", op);

	data = json_object();
	json_object_set(obj, "data", data);
	json_decref(data);
	return data;
}

#endif
