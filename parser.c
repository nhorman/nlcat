#include <string.h>
#include <parser.h>

static struct json_map* copy_subtree(struct json_map *parent)
{
	struct json_map *new;
	size_t i;

	if (!parent->num_children)
		return NULL;
	new = malloc(sizeof(struct json_map) * parent->num_children);
	memcpy(new, parent->children, sizeof(struct json_map) * parent->num_children);
	for(i=0; i < parent->num_children; i++)
		new[i].children = copy_subtree(&new[i]);
	return new; 
}

struct json_map* clone_json_template(struct json_map *tmpl)
{
	struct json_map *new = malloc(sizeof(struct json_map));
	memcpy(new, tmpl, sizeof(struct json_map));
	new->children = copy_subtree(new);
	return new;
}

static void free_subtree(struct json_map *clone)
{
	size_t i;
	if (!clone)
		return;
	for (i=0; i<clone->num_children; i++)
		free_subtree(&clone[i]);
	free(clone->children);
}

void free_json_template(struct json_map *clone)
{
	free_subtree(clone);
	free(clone);
	return;
}
