/*
 * Component simulator main header
 * Copyright (c) 2019, IRIT - UPS <casse@irit.fr>
 *
 * This file is part of GLISS2.
 *
 * GLISS2 is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * GLISS2 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GLISS2; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <stdlib.h>
#include <string.h>

#define CSIM_INSIDE
#include "mem.h"
#include "csim.h"

#include "yaml.h"

#include "led.h"
#include "button.h"
#include "seven_seg_controller.h"
#include "seven_seg_display.h"
#include "leds10.h"
#include "timer.h"
#include "arm_core.h"


/**
 * Available components.
 */
csim_component_t *comps[] = {
	&led_component.comp,
	&button_component.comp,
	&seven_seg_controller_component,
	&timer_component,
	&seven_seg_display_component.comp,
	&leds10_component,
	&arm_component.comp,
	NULL
};

/**
 * Loader structure for YAML parsing.
 */
typedef struct {
	enum {
		TOP,
		IN_COMPS,
		IN_COMP,
		IN_CONNECT,
		IN_LINK
	} state;
	csim_board_t *board;
	csim_memory_t *mem;
	const char *name, *type;
	char key;
	arm_address_t base;
	csim_inst_t *from_inst, *to_inst;
	csim_port_t *from_port, *to_port;
	int conf_cnt;
	char *confs[32];
} loader_t;

/**
 * Scan a port in the form INSTANCE.PORT.
 * @param loader	Current loader.
 * @param val		YAML value to scan.
 * @param inst		Result instance.
 * @param port		Result port.
 * @return			Next action for YAML.
 */
yaml_next_t scan_port(loader_t *loader, const char *val, csim_inst_t **inst, csim_port_t **port) {
	if(*inst != NULL)
		return YAML_ERROR;

	// find '.'
	char buf[64];
	strncpy(buf, val, 64);
	char *p = strchr(buf, '.');
	if(p == NULL) {
		fprintf(stderr, "ERROR: '.' is missing.\n");
		return YAML_ERROR;
	}
	*p = '\0';

	// find instance
	for(csim_inst_t *i = loader->board->insts; i != NULL; i = i->next)
		if(strcmp(i->name, buf) == 0) {
			*inst = i;
			break;
		}
	if(*inst == NULL) {
		fprintf(stderr, "ERROR: cannot find instance %s.\n", buf);
		return YAML_ERROR;
	}

	// find port
	for(unsigned i = 0; i < (*inst)->comp->port_cnt; i++) {
		if(strcmp(p + 1, (*inst)->comp->ports[i].name) == 0) {
			*port = &(*inst)->comp->ports[i];
			break;
		}
	}
	if(*port == NULL) {
		fprintf(stderr, "ERROR: cannot find port %s.\n", p +1);
		return YAML_ERROR;
	}

	// all is fine
	return  YAML_DONE;
}


///
static yaml_next_t on_key(const char *key, const char *val, void *data) {
	loader_t *loader = (loader_t *)data;
	switch(loader->state) {

	case TOP:
		if(strcmp(key, "name") == 0) {
			loader->name = strdup(val);
			return YAML_DONE;
		}
		else if(strcmp(key, "components") == 0) {
			loader->board = csim_new_board(loader->name, loader->mem);
			loader->board->level = CSIM_ERROR;
			loader->state = IN_COMPS;
			return YAML_MAP;
		}
		else if(strcmp(key, "connect") == 0) {
			loader->state = IN_CONNECT;
			return YAML_LIST;
		}
		break;

	case IN_COMPS:
		loader->name = strdup(key);
		loader->state = IN_COMP;
		return YAML_MAP;

	case IN_COMP:
		if(strcmp(key, "type") == 0) {
			loader->type = strdup(val);
			return YAML_DONE;
		}
		else if(strcmp(key, "base") == 0) {
			sscanf(val, "%x", &loader->base);
			return YAML_DONE;
		}
		else {
			loader->confs[loader->conf_cnt++] = strdup(key);
			loader->confs[loader->conf_cnt++] = strdup(val);
			return YAML_DONE;
		}
		break;

	case IN_LINK:
		if(strcmp(key, "from"))
			return scan_port(loader, val, &loader->from_inst, &loader->from_port);
		else if(strcmp(key, "to"))
			return scan_port(loader, val, &loader->to_inst, &loader->to_port);
		break;

	default:
		break;
	}
	return YAML_ERROR;
}


static yaml_next_t on_item(const char *val, void *data) {
	loader_t *loader = (loader_t *)data;
	loader->state = IN_LINK;
	return YAML_MAP;
}


///
static void on_end(void *data) {
	loader_t *loader = (loader_t *)data;

	switch(loader->state) {

	case TOP:
		break;

	case IN_COMP: {

			/* find the component */
			csim_component_t *type = NULL;

			/* build the component */
			for(int i = 0; comps[i] != NULL; i++)
				if(strcmp(loader->type, comps[i]->name) == 0) {
					type = comps[i];
					break;
				}
			if(type == NULL) {
				fprintf(stderr, "ERROR: component type %s does not exist!\n", loader->type);
				exit(1);
			}

			/* buiild the component */
			loader->confs[loader->conf_cnt] = NULL;
			csim_new_component_ext(loader->board, type, loader->name, loader->base, loader->confs);
			for(int i = 0; i < loader->conf_cnt; i++)
				free(loader->confs[i]);

			loader->state = IN_COMPS;
		}
		break;

	case IN_COMPS:
		loader->state = TOP;
		break;

	case IN_LINK:
		csim_connect(loader->from_inst, loader->from_port, loader->to_inst, loader->to_port);
		loader->from_inst = NULL;
		loader->from_port = NULL;
		loader->to_inst = NULL;
		loader->to_port = NULL;
		loader->state = IN_CONNECT;
		break;

	default:
		break;
	}
}

/**
 * Load the content of a board from given file.
 *
 * In case of error, display it and stop the program.
 * @param path		Path to read board from.
 * @param mem		Memory to use for I/O registers.
 * @return			Created board or NULL if there is an error.
 */
csim_board_t *csim_load_board(const char *path, csim_memory_t *mem) {
	loader_t loader = {
		TOP, NULL, mem, "anonymous", NULL, '\0', 0,
		NULL, NULL, NULL, NULL,
		0, { NULL }
	};
	yaml_handler_t handler;
	yaml_init_handler(&handler);
	handler.on_key = on_key;
	handler.on_item = on_item;
	handler.on_end = on_end;
	int num = yaml_parse(&handler, path, &loader);
	if(num == 0)
		return loader.board;
	else {
		csim_delete_board(loader.board);
		return NULL;
	}
}
