#include <stdio.h>
#include <errno.h>
#include <string.h>
#include "yaml.h"

///
static yaml_next_t yaml_on_key(const char *key, const char *value, void *data) {
	return YAML_ERROR;
}

///
static yaml_next_t yaml_on_item(const char *value, void *data) {
	return YAML_ERROR;
}

///
static void yaml_on_end(void *data) {}

///
static void yaml_on_error(const char *msg, void *data) {
	fprintf(stderr, "ERROR: %s\n", msg);
}

/**
 * Initialize the given handler with default value. Then handler can then be
 * customized by the user.
 *
 * As a default, on_key() and on_value() return an error. on_erro() does nothing
 * and on_error() displays the error message to stderr.
 *
 * @param handler	Handler to initialize.
 */
void yaml_init_handler(yaml_handler_t *handler) {
	handler->on_key = yaml_on_key;
	handler->on_item = yaml_on_item;
	handler->on_end = yaml_on_end;
	handler->on_error = yaml_on_error;
}

/**
 * Parse the given path as a YAML format using the given handler.
 * @param handler	Handler to emit foudn values.
 * @param path		Path to the file to open.
 * @param data		Date to pass to the handler functions.
 * @return			0 if all is fine, -1 or line number if there is an error.
 */
int yaml_parse(yaml_handler_t *handler, const char *path, void *data) {
	char buf[256];
	typedef enum {
		IN_MAP,
		IN_LIST
	} state_t;
	state_t state = IN_MAP;
	state_t stack[32];
	int stack_top = 0;
	int num = 0;

	/* open the file */
	FILE *in = fopen(path, "r");
	if(in == NULL) {
		handler->on_error(strerror(errno), data);
		return -1;
	}

	/* traverse the file */
	while(fgets(buf, sizeof(buf), in) != NULL) {
		num++;

		/* count spaces and ignore empty lines */
		int cnt = 0;
		while(buf[cnt] == ' ') cnt++;
		if(buf[cnt] == '\n' || buf[cnt] == '\0')
			continue;

		/* process different levels of spaces */
		if((cnt & 1) == 1) {
			sprintf(buf, "%d: odd number of spaces.", num);
			handler->on_error(buf, data);
			return num;
		}
		if(cnt > stack_top*2) {
			sprintf(buf, "%d: unexpected number of spaces.", num);
			handler->on_error(buf, data);
			return num;
		}
		while(cnt < stack_top*2) {
			handler->on_end(data);
			stack_top--;
			state = stack[stack_top];
		}

		/* remove '\n' */
		char *p = buf + cnt;
		int len = strlen(p);
		if(p[len - 1] == '\n') {
			p[len - 1] = '\0';
			len--;
		}

		/* process the line */
		yaml_next_t next;
		switch(state) {

		case IN_MAP: {
				char *q = strchr(buf, ':');
				if(q == NULL)
					q = p + len - 2;
				else if(*(q + 1) == '\0') {
					*q = '\0';
					q -=2;
				}
				else if(*(q + 1) == ' ')
					*q = '\0';
				else {
					sprintf(buf, "%d: space after ':' required!", num);
					handler->on_error(buf, data);
					return num;
				}
				next = handler->on_key(p, q+2, data);
			}
			break;

		case IN_LIST: {
				if(*p != '-' && *(p + 1) != ' ') {
					sprintf(buf, "%d: list required here.", num);
					handler->on_error(buf, data);
					next = YAML_ERROR;
				}
				else
					next = handler->on_item(p + 2, data);
			}
			break;

		default:
			next = YAML_ERROR;
			break;
		}

		switch(next) {
		case YAML_DONE:
			break;
		case YAML_ERROR:
			return num;
		case YAML_MAP:
			stack[stack_top++] = state;
			state = IN_MAP;
			break;
		case YAML_LIST:
			stack[stack_top++] = state;
			state = IN_LIST;
			break;
		}
	}

	/* terminate */
	if(feof(in)) {
		num = 0;
		handler->on_end(data);
		while(stack_top > 0) {
			stack_top--;
			handler->on_end(data);
		}
	}
	else
		handler->on_error(strerror(errno), data);
	fclose(in);
	return num;
}
