/*
 * Component simulator test program
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
#include <stdio.h>

/*#define CSIM_INSIDE
#include "mem.h"*/
#include "csim.h"


/****** Component c1 ******/
extern csim_component_t c1;

void e1_trigger(csim_evt_t *evt) {
	csim_board_t *b = evt->inst->board;
	b->log(b, CSIM_INFO, "e1 triggered!");
	static int cnt = 0;
	csim_send_digital(evt->inst, &c1.ports[0], cnt++);
}

csim_evt_t e1 = {
	NULL,
	NULL,
	10,
	10,
	NULL,
	e1_trigger
};

void c1_construct(csim_inst_t *i, csim_confs_t confs) {
	e1.inst = i;
	csim_record_event(i->board, &e1);
}

void c1_destruct(csim_inst_t *i) {
}

void c1_reset(csim_inst_t *i) {
}

void p1_receive(csim_port_inst_t *port, csim_value_type_t type, csim_value_t val) { }

csim_port_t c1_ports[] = {
	 { "p1", CSIM_ELECTRIC, p1_receive }
};

void c1_write(csim_inst_t *i, int n, csim_word_t v)
	{ i->board->log(i->board, CSIM_INFO, "c1_write(%d, %d)", n, v); }
csim_word_t c1_read(csim_inst_t *i, int n)
	{ i->board->log(i->board, CSIM_INFO, "c1_read(%d)", n);  return 666; }

csim_reg_t c1_regs[] = {
	{ "R", 0x100, 4, 1, 1, 0, CSIM_INT, NULL, NULL, c1_read, c1_write, NULL, NULL }
};

csim_component_t c1 = {
	"C1",
	CSIM_SIMPLE,
	1,
	c1_regs,
	1,
	c1_ports,
	1,
	sizeof(csim_inst_t),
	c1_construct,
	c1_destruct,
	c1_reset
};


/****** Component c2 ******/

extern csim_component_t c2;

void c2_construct(csim_inst_t *c, csim_confs_t confs) {
}

void c2_destruct(csim_inst_t *c) {
}

void c2_reset(csim_inst_t *c) {
}

void p2_receive(csim_port_inst_t *port, csim_value_type_t type, csim_value_t val) {
	csim_board_t *b = port->inst->board;
	b->log(b, CSIM_INFO, "receive %d at %s of %s", val, port->port->name, port->inst->name);
}

csim_port_t c2_ports[] = {
	{ "p2", CSIM_ELECTRIC, p2_receive }
};


csim_component_t c2 = {
	"C2",
	CSIM_SIMPLE,
	1,
	NULL,
	0,
	c2_ports,
	1,
	sizeof(csim_inst_t),
	c2_construct,
	c2_destruct,
	c2_reset
};

int main() {
	csim_board_t *board = csim_new_board("my-board");
	board->level = CSIM_DEBUG;

	csim_inst_t *i1 = csim_new_component(board, &c1, "i1", 0x80000000);
	csim_inst_t *i2 = csim_new_component(board, &c2, "i2", 0x90000000);
	csim_connect(i1, &c1_ports[0], i2, &c2_ports[0]);

	csim_run(board, 50);

	//csim_mem_write32(mem, 0x80000100, 111);
	//printf("read: %d\n", csim_mem_read32(mem, 0x80000100));

	csim_connect(i1, &c1_ports[0], i2, &c2_ports[0]);
	csim_delete_board(board);
	//csim_mem_delete(mem);
	return 0;
}
