#include "led.h"

/**
 * @defgroup led LED Component
 *
 * Represent a LED with a ON/OFF behaviour.
 *
 * Ports:
 * * input (input, digital) -- LED input.
 */

typedef struct led_inst_t {
	csim_iocomp_inst_t inst;
	int state;
} led_inst_t;

void led_reset(csim_inst_t *inst) {
	led_inst_t *i = (led_inst_t *)inst;
	i->state = 0;
}

/**
 * @ingroup led
 */
void led_construct(csim_inst_t *inst, csim_confs_t confs) {
	led_reset(inst);
}

/**
 * @ingroup led
 */
void led_destruct(csim_inst_t *i) { }

/**
 * @ingroup led
 */
void led_update(csim_port_inst_t *inst, csim_value_type_t type, csim_value_t val) {
	led_inst_t *i = (led_inst_t *)inst->inst;
	i->state = val.digital;
}

/**
 * @ingroup led
 */
csim_port_t led_ports[] = {
	{ "input", CSIM_DIGITAL, led_update }
};

/**
 * @ingroup led
 */
void led_write(csim_inst_t *inst, int n, csim_word_t v) {
	led_inst_t *i = (led_inst_t *)inst;
	i->state = v;
	inst->board->log(inst->board, CSIM_DEBUG, "led_write(%d, %d)", n, v);
}

/**
 * @ingroup led
 */
csim_word_t led_read(csim_inst_t *inst, int n) {
	led_inst_t *i = (led_inst_t *)inst;
	inst->board->log(inst->board, CSIM_DEBUG, "led_read(%d)", n);
	return i->state;
}

csim_reg_t led_regs[] = {
	{ "IN", 0, 4, 1, 1, 0, CSIM_INT, NULL, NULL, led_read, led_write, NULL, NULL }
};

/**
 * @ingroup led
 */
static int led_display(char *buf, csim_iocomp_inst_t *inst) {
	led_inst_t *i = (led_inst_t *)inst;
	if(i->state)
		return sprintf(buf, "[*]");
	else
		return sprintf(buf, "[ ]");
}

/**
 * @ingroup led
 */
static void led_on_key(char key, csim_iocomp_inst_t *inst) {
}

void led_get_state(csim_iocomp_inst_t *inst, uint32_t *state) {
	led_inst_t *i = (led_inst_t *)inst;
	*state = i->state;
}

void led_set_state(csim_iocomp_inst_t *inst, uint32_t *state) {
	led_inst_t *i = (led_inst_t *)inst;
	if(*state != i->state)
		i->state = *state;
}

/**
 * @ingroup led
 */
csim_iocomp_t led_component = {
	{
		"led",
		CSIM_IO,
		1,
		led_regs,
		1,
		led_ports,
		1,
		sizeof(led_inst_t),
		led_construct,
		led_destruct,
		led_reset,
		csim_default_update
	},
	led_display,
	led_on_key,
	led_get_state,
	led_set_state
};

/**
 * @ingroup led
 */
int led_state(csim_inst_t *inst) {
	led_inst_t *i = (led_inst_t *)inst;
	return i->state;
}

