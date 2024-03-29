#include <string.h>

#include "button.h"

typedef struct button_inst_t {
	csim_iocomp_inst_t inst;
	int pushed;
	char key;
} button_inst_t;

void button_reset(csim_inst_t *inst) {
	button_inst_t *i = (button_inst_t *)inst;
	i->pushed = 0;
	i->key = '\0';
}

void button_construct(csim_inst_t *inst, csim_confs_t confs) {
	button_inst_t *i = (button_inst_t *)inst;
	button_reset(inst);
	for(int j = 0; confs[j] != NULL; j += 2) {
		printf("DEBUG: arg %s=%s\n", confs[j], confs[j+1]);
		if(strcmp(confs[j], "key") == 0) {
			sscanf(confs[j + 1], "%c", &i->key);
			printf("DEBUG: %s -> %c\n", confs[j+1], i->key);
		}
	}
}

void button_destruct(csim_inst_t *c) {
}

void button_write(csim_inst_t *inst, int n, csim_word_t v) {
	inst->board->log(inst->board, CSIM_INFO, "button_write(%d, %d)", n, v);
}

csim_word_t button_read(csim_inst_t *inst, int n) {
	button_inst_t *i = (button_inst_t *)inst;
	inst->board->log(inst->board, CSIM_INFO, "button_read(%d)", n);
	return i->pushed;
}

csim_reg_t button_regs[] = {
	{ "R", 0, 4, 1, 1, 0, CSIM_INT, NULL, NULL, button_read, button_write, NULL, NULL }
};

void button_update(csim_port_inst_t *port, csim_value_type_t type, csim_value_t val) {
}

csim_port_t button_ports[] = {
	{ "output", CSIM_DIGITAL, button_update }
};

static int button_display(char *buf, csim_iocomp_inst_t *inst) {
	button_inst_t *i = (button_inst_t *)inst;
	if(i->pushed)
		return sprintf(buf, ")%c(", i->key);
	else
		return sprintf(buf, "(%c)", i->key);
}

static void button_on_key(char key, csim_iocomp_inst_t *inst) {
	button_inst_t *i = (button_inst_t *)inst;
	if(key == i->key) {
		i->pushed = !i->pushed;
		csim_send_digital(&inst->inst, &button_ports[0], i->pushed);
	}
}

csim_iocomp_t button_component = {
	{
		"button",
		CSIM_IO,
		1,
		button_regs,
		1,
		button_ports,
		1,
		sizeof(button_inst_t),
		button_construct,
		button_destruct,
		button_reset
	},
	button_display,
	button_on_key
};

int button_get(csim_inst_t *inst) {
	button_inst_t *i = (button_inst_t *)inst;
	return i->pushed;
}

void button_set(csim_inst_t *inst, int pushed) {
	button_inst_t *i = (button_inst_t *)inst;
	if(pushed != i->pushed) {
		i->pushed = pushed;
		csim_send_digital(inst, &button_ports[0], pushed);
	}
}

