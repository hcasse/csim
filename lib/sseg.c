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

#include "sseg.h"

///
typedef struct sseg_inst_t {
	csim_iocomp_inst_t inst;
	uint8_t state;
} sseg_inst_t;

///
static csim_iostate_t change = { NULL, { 0, SSEG_SEGS, 0 }};

///
static void sseg_reset(csim_inst_t *inst) {
	sseg_inst_t *i = (sseg_inst_t *)inst;
	i->state = 0;
	change.info.state = 0;
	csim_record_iostate(inst, &change);
}

///
static void sseg_construct(csim_inst_t *inst, csim_confs_t confs) {
	sseg_reset(inst);
	change.info.id = inst->id;
}

///
static void sseg_destruct(csim_inst_t *i) { }

///
static void sseg_set(csim_port_inst_t *inst, int num, int val) {
	sseg_inst_t *i = (sseg_inst_t *)inst->inst;
	if(val)
		i->state |= 1 << num;
	else
		i->state &= ~(1 << num);
	csim_wakeup(inst->inst);
}

///
static void sseg_on_up(csim_port_inst_t *inst, csim_value_type_t type, csim_value_t val) {
	sseg_set(inst, 0, val.digital);
}

///
static void sseg_on_upleft(csim_port_inst_t *inst, csim_value_type_t type, csim_value_t val) {
	sseg_set(inst, 1, val.digital);
}

///
static void sseg_on_upright(csim_port_inst_t *inst, csim_value_type_t type, csim_value_t val) {
	sseg_set(inst, 2, val.digital);
}

///
static void sseg_on_mid(csim_port_inst_t *inst, csim_value_type_t type, csim_value_t val) {
	sseg_set(inst, 3, val.digital);
}

///
static void sseg_on_downleft(csim_port_inst_t *inst, csim_value_type_t type, csim_value_t val) {
	sseg_set(inst, 4, val.digital);
}

///
static void sseg_on_downright(csim_port_inst_t *inst, csim_value_type_t type, csim_value_t val) {
	sseg_set(inst, 5, val.digital);
}

///
static void sseg_on_down(csim_port_inst_t *inst, csim_value_type_t type, csim_value_t val) {
	sseg_set(inst, 6, val.digital);
}

///
csim_port_t sseg_ports[] = {
	{ "UP", CSIM_DIGITAL, sseg_on_up },
	{ "UPLEFT", CSIM_DIGITAL, sseg_on_upleft },
	{ "UPRIGHT", CSIM_DIGITAL, sseg_on_upright },
	{ "MID", CSIM_DIGITAL, sseg_on_mid },
	{ "DOWNLEFT", CSIM_DIGITAL, sseg_on_downleft },
	{ "DOWNRIGHT", CSIM_DIGITAL, sseg_on_downright },
	{ "DOWN", CSIM_DIGITAL, sseg_on_down },
};

///
static csim_reg_t sseg_regs[] = {
};

///
static int sseg_display(char *buf, csim_iocomp_inst_t *inst) {
	sseg_inst_t *i = (sseg_inst_t *)inst;
	char c = '?';
	switch(i->state) {
	case 0b0000000:	c = ' '; break;
	case SSEG_0: 	c = '0'; break;
	case SSEG_1: 	c = '1'; break;
	case SSEG_2: 	c = '2'; break;
	case SSEG_3: 	c = '3'; break;
	case SSEG_4: 	c = '4'; break;
	case SSEG_5: 	c = '5'; break;
	case SSEG_6: 	c = '6'; break;
	case SSEG_7: 	c = '7'; break;
	case SSEG_8: 	c = '8'; break;
	case SSEG_9: 	c = '9'; break;
	}
	return sprintf(buf, "[%c]", c);
}

///
static void sseg_on_key(char key, csim_iocomp_inst_t *inst) {
}

///
static void sseg_get_state(csim_iocomp_inst_t *inst, uint32_t *state) {
	sseg_inst_t *i = (sseg_inst_t *)inst;
	*state = i->state;
}

///
static void sseg_set_state(csim_iocomp_inst_t *inst, uint32_t *state) {
	sseg_inst_t *i = (sseg_inst_t *)inst;
	if(*state != i->state)
		i->state = *state;
}

///
static void sseg_update(csim_inst_t *inst) {
	sseg_inst_t *i = (sseg_inst_t *)inst;
	change.info.state = i->state;
	csim_record_iostate(inst, &change);
}

/**
 * Descriptor of a 7-segment component.
 * @ingroup comp
 */
csim_iocomp_t sseg_component = {
	{
		NULL,
		"sseg",
		CSIM_IO,
		1,
		sseg_regs,
		0,
		sseg_ports,
		7,
		sizeof(sseg_inst_t),
		sseg_construct,
		sseg_destruct,
		sseg_reset,
		sseg_update
	},
	sseg_display,
	sseg_on_key,
	sseg_get_state,
	sseg_set_state
};
