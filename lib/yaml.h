#ifndef YAML_H
#define YAML_H

typedef enum yaml_next_t {
	YAML_DONE,
	YAML_ERROR,
	YAML_LIST,
	YAML_MAP
} yaml_next_t;

typedef struct {
	yaml_next_t (*on_key)(const char *key, const char *value, void *data);
	yaml_next_t (*on_item)(const char *value, void *data);
	void (*on_end)(void *data);
	void (*on_error)(const char *msg, void *data);
} yaml_handler_t;

void yaml_init_handler(yaml_handler_t *handler);
int yaml_parse(yaml_handler_t *handler, const char *path, void *data);

#endif	// YAML_H
