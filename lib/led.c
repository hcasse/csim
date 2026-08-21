/*
 * CSIM -- Component simulator
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

static csim_iostate_t shine = { NULL, { 0, LED_SHINE, 0 }};

///
static void led_reset(csim_inst_t *inst) {
	led_inst_t *i = (led_inst_t *)inst;
	if(i->state != 0) {
		i->state = 0;
		shine.info.state = 0;
		csim_record_iostate(inst, &shine);
	}
}

///
static void led_construct(csim_inst_t *inst, csim_confs_t confs) {
	led_reset(inst);
	shine.info.id = inst->id;
}

///
static void led_destruct(csim_inst_t *i) { }

///
static void led_on_port(csim_port_inst_t *inst, csim_value_type_t type, csim_value_t val) {
	led_inst_t *i = (led_inst_t *)inst->inst;
	if(val.digital != i->state) {
		i->state = val.digital;
		csim_wakeup(inst->inst);
	}
}

///
csim_port_t led_ports[] = {
	{ "IN", CSIM_DIGITAL, led_on_port }
};

///
static void led_write(csim_inst_t *inst, int n, csim_word_t val) {
	led_inst_t *i = (led_inst_t *)inst;
	if(i->state != val) {
		i->state = val;
		inst->board->log(inst->board, CSIM_DEBUG, "led_write(%d, %d)", n, val);
		csim_wakeup(inst);
	}
}

///
static csim_word_t led_read(csim_inst_t *inst, int n) {
	led_inst_t *i = (led_inst_t *)inst;
	inst->board->log(inst->board, CSIM_DEBUG, "led_read(%d)", n);
	return i->state;
}

csim_reg_t led_regs[] = {
	{ "IN", 0, 4, 1, 1, 0, CSIM_INT, NULL, NULL, led_read, led_write, NULL, NULL }
};

///
static int led_display(char *buf, csim_iocomp_inst_t *inst) {
	led_inst_t *i = (led_inst_t *)inst;
	if(i->state)
		return sprintf(buf, "[*]");
	else
		return sprintf(buf, "[ ]");
}

///
static void led_on_key(char key, csim_iocomp_inst_t *inst) {
}

///
static void led_get_state(csim_iocomp_inst_t *inst, uint32_t *state) {
	led_inst_t *i = (led_inst_t *)inst;
	*state = i->state;
}

///
static void led_set_state(csim_iocomp_inst_t *inst, uint32_t *state) {
	led_inst_t *i = (led_inst_t *)inst;
	if(*state != i->state)
		i->state = *state;
}

///
static void led_update(csim_inst_t *inst) {
	led_inst_t *i = (led_inst_t *)inst;
	shine.info.state = i->state;
	csim_record_iostate(inst, &shine);
}

/**
 * Descriptor of a LED component.
 * @ingroup comp
 */
csim_iocomp_t led_component = {
	{
		-1,
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
		led_update
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

