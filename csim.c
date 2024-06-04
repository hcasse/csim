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

#include <assert.h>
#include <memory.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CSIM_INSIDE
#include "mem.h"
#include "csim.h"

/**
 * @defgroup csim Simulation Module
 *
 * Main module of CSIM.
 *
 * @author H. Cassé
 *
 */

/**
 * @typedef csim_reg_t
 * Describe a register.
 * @ingroup csim
 * 
 * @var csim_reg_t::name
 * Name of the register.
 * @var csim_reg_t::offset
 * Offset of the register (relative to base address of the component).
 * @var csim_reg_t::size
 * Size in bits of the register.
 * @var csim_reg_t::count
 * Number of registers for an array of registers.
 * @var csim_reg_t::stride
 * Distance, in bytes, between two entries of a register array (usually size of the regioster).
 * @var csim_reg_t::type
 * One of CSIM_BITS, SIM_INT, CSIM_ADDR, CSIM_FLOAT32, CSIM_FLOAT64.
 * @var csim_reg_t::make_name
 * Function called to generate the name of the register (for user).
 * @var csim_reg_t::display
 * Function to display the current value of the register.
 * @var csim_reg_t::read
 * Function called when the register is read by the simulator throught memory.
 * @var csim_reg_t::write
 * Function called when the register is written by the simulator throught memory.
 * @var csim_reg_t::get
 * Function called when the register is read out of simulation scope.
 * @var csim_reg_t::set
 * Function called when the register is written out of simulation scope.
 */

/**
 * @typedef csim_port_t
 * Represents the instanciation of port (from an intanciated component)
 * and can be linked. Contains also the current value on the connection.
 * @ingroup csim
 *
 * @var csim_port_t::port
 * Port of the instance.
 * @var csim_port_t::inst
 * Component instance owning the port.
 * @var csim_port_t::link
 * Link to the connected port instance.
 */

#define CSIM_IO_HASH(a)	(((csim_word_t)(a) >> CSIM_IO_SHIFT) & (CSIM_IO_SIZE - 1))

/**
 * Default log function: log to stderr.
 * @param board	Current board.
 * @param level	Logging level.
 * @param msg	Message to display.
 * @ingroup csim
 */
void csim_log(csim_board_t *board, csim_level_t level, const char *msg, ...) {
	static const char *pref[] = {
		"",
		"DEBUG  ",
		"INFO   ",
		"WARNING",
		"ERROR  ",
		"FATAL  "
	};

	if(level < board->level)
		return;

	va_list args;
	va_start(args, msg);
	fprintf(stderr, "%s: % 9ld: ", pref[level], board->date);
	vfprintf(stderr, msg, args);
	fputc('\n', stderr);
	va_end(args);
}


/* unit management */
struct {
	const char *name;
	csim_port_type_t type;
} csim_units[256] = {
	{ "volt", CSIM_ELECTRIC }
};
int csim_unit_top = 0;


/**
 * Get identifier for a new or existing unit based on its name.
 * @param name	Unit name.
 * @return		Corresponding unit type.
 * @ingroup csim
 */
csim_port_type_t csim_get_unit(const char *name) {
	for(int i = 0; i <= csim_unit_top; i++)
		if(strcmp(name, csim_units[i].name) == 0)
			return csim_units[i].type;
	csim_unit_top++;
	csim_units[csim_unit_top].name = name;
	csim_units[csim_unit_top].type = csim_unit_top;
	return csim_unit_top;
}


/**
 * Get the name of a unit type.
 * @param t		Type to get name for.
 * @return		Type name.
 */
const char *csim_unit_name(csim_port_type_t t) {
	assert(t <= csim_unit_top);
	return csim_units[t].name;
}


/**
 * Called to manage an IO.
 * @param addr		Accessed address.
 * @param size		Accessed size.
 * @param data		Pointer to data.
 * @param access	Type of access.
 * @param cdata		Should be board.
 * @ingroup csim
 */
static void csim_on_io(csim_addr_t addr, int size, void *data, int access, void *cdata) {
	csim_board_t *board = (csim_board_t *)cdata;
	int h = CSIM_IO_HASH(addr);
	for(csim_io_t *p = board->ios[h]; p != NULL; p = p->next)
		if(p->addr == addr) {
			int i = (addr - p->inst->base - p->reg->offset) / p->reg->stride;
			if(size != p->reg->size)
				board->log(board, CSIM_ERROR, "bad IO access at %08x: size=%d and should be %d", addr, size, p->reg->size);
			else
				switch(p->reg->size) {
					
				case 1:
					if(access == CSIM_MEM_READ)
						*(int8_t *)data = p->reg->read(p->inst, i);
					else
						p->reg->write(p->inst, i, *(int8_t *)data);
					break;
				
				case 2:
					if(access == CSIM_MEM_READ)
						*(int16_t *)data = p->reg->read(p->inst, i);
					else
						p->reg->write(p->inst, i, *(int16_t *)data);
					break;
				
				case 4:
					if(access == CSIM_MEM_READ)
						*(int32_t *)data = p->reg->read(p->inst, i);
					else
						p->reg->write(p->inst, i, *(int32_t *)data);
					break;
				
				default:
					assert(0);
				}
		}
}


/**
 * Add IO entry for the given registers in the given instance.
 * @param reg	Register to record.
 * @param inst	Component instance.
 * @ingroup csim
 */
void csim_io_add(csim_reg_t *reg, csim_inst_t *inst) {
	csim_board_t *board = inst->board;
	for(int i = 0; i < reg->count; i++) {
		
		/* build the IO entry */
		csim_addr_t a = inst->base + i * reg->stride + reg->offset;
		int h = CSIM_IO_HASH(a);
		csim_io_t *io = (csim_io_t *)malloc(sizeof(csim_io_t));
		io->addr = a;
		io->reg = reg;
		io->inst = inst;
		io->next = board->ios[h];
		board->ios[h] = io;
		
		/* install the IO in memory */
		csim_addr_t pa = a & ~(CSIM_PAGE_SIZE - 1);
		if(csim_get_callback_data(board->mem, pa) == NULL)
			csim_set_range_callback(board->mem, pa, pa + reg->size - 1, csim_on_io, board);
	}
}


/**
 * Remove IO entries for the given register.
 * @param reg	Register to record.
 * @param inst	Component instance.
 * @ingroup csim
 */
void csim_io_remove(csim_reg_t * reg, csim_inst_t *inst) {
	csim_board_t *board = inst->board;
	for(int i = 0; i < reg->count; i++) {
		csim_addr_t a = inst->base + i * reg->stride + reg->offset;
		int h = CSIM_IO_HASH(a);
		for(csim_io_t *p = board->ios[h], *q = NULL; p != NULL; q = p, p = p->next)
			if(p->addr == a) {
				if(q == NULL)
					board->ios[h] = p->next;
				else
					q->next = p->next;
				free(p);
			}
		assert(0);
	}
}


/**
 * Initialize the board.
 * @param name	Board name.
 * @param mem	Memory to use (possibly NULL and then derived from core).
 * @return		Built board (or null if allocation fails).
 * @ingroup csim
 */
csim_board_t *csim_new_board(const char *name, csim_memory_t *mem) {
	csim_board_t *board = (csim_board_t *)malloc(sizeof(csim_board_t));
	if(board == NULL)
		return NULL;
	board->name = name;
	board->insts = NULL;
	board->cores = NULL;
	board->iocomps = NULL;
	board->clock = 0;
	board->date = 0;
	board->evts = NULL;
	board->level = CSIM_INFO;
	board->mem = mem;
	board->log = csim_log;
	memset(board->ios, 0, sizeof(csim_io_t *) * CSIM_IO_SIZE);
	return board;
}

/**
 * Delete the given board.
 * @param board		Board to delete.
 * @ingroup csim
 */
void csim_delete_board(csim_board_t *board) {
	
	csim_inst_t *i = board->insts;
	while(i != NULL) {
		csim_inst_t *next = i->next;
		csim_delete_component(i);
		i = next;
	}
	
	for(int i = 0; i < CSIM_IO_SIZE; i++) {
		csim_io_t *p = board->ios[i];
		while(p != NULL) {
			csim_io_t *q = p->next;
			free(p);
			p = q;
		}
	}

	board->log(board, CSIM_INFO, "deleting board %s", board->name);
	free(board);
}


/**
 * Build a new instance of the given component and add it to the board.
 * @param board	Board to add to.
 * @param comp	Component to build an instance for.
 * @param name	Name of the instance.
 * @param base	Base adress of the instance.
 * @return		Built instance.
 * @ingroup csim
 */
csim_inst_t *csim_new_component(csim_board_t *board, csim_component_t *comp, const char *name, csim_addr_t base) {
	csim_confs_t confs = { NULL };
	return csim_new_component_ext(board, comp, name, base, confs);
}

/**
 * Record the registers of the component in the address space.
 * @param board		Current board.
 * @param inst		Component instance.
 */
static void csim_record_regs(csim_board_t *board, csim_inst_t *inst) {
	csim_component_t *comp = inst->comp;
	for(int j = 0; j < comp->reg_cnt; j++)
		csim_io_add(&comp->regs[j], inst);
}

/**
 * Build a new instance of the given component and add it to the board.
 *
 * confs is a null-terminated array of strings organized by pairs which
 * first member is the entry name and the second the entry value.
 * 
 * @param board	Board to add to.
 * @param comp	Component to build an instance for.
 * @param name	Name of the instance.
 * @param base	Base adress of the instance.
 * @param confs	Configurations.
 * @return		Built instance.
 * @ingroup csim
 */
csim_inst_t *csim_new_component_ext(csim_board_t *board, csim_component_t *comp, const char *name, csim_addr_t base, csim_confs_t confs) {

	/* build the instance */
	uint8_t *p = (uint8_t *)malloc(comp->size + comp->port_cnt * sizeof(csim_port_inst_t));
	csim_inst_t *i = (csim_inst_t *)p;
	i->comp = comp;
	i->name = strdup(name);
	i->base = base;
	i->board = board;
	i->ports = (csim_port_inst_t *)(p + comp->size);
	for(int j = 0; j < comp->port_cnt; j++) {
		i->ports[j].port = &comp->ports[j];
		i->ports[j].inst = i;
		i->ports[j].link = NULL;
	}
	i->number = 0;
	for(csim_inst_t *p = board->insts; p != NULL; p = p->next)
		if(p->comp == comp)
			i->number++;

	/* link to the board */
	i->next = board->insts;
	board->insts = i;

	/* if core, record it in core list */
	if(comp->type == CSIM_CORE) {
		csim_core_inst_t *ci = (csim_core_inst_t *)i;
		csim_core_t *cc = (csim_core_t *)comp;
		ci->next = board->cores;
		board->cores = ci;
		if(board->clock == 0)
			board->clock = cc->clock;
		else if(cc->clock != 0) {
			if(board->clock != cc->clock)
				board->log(board, CSIM_FATAL, "ERROR: current version only supports multiple core with same clock.");
		}
	}

	/* If IO component, record it in the IO list. */
	if(comp->type == CSIM_IO) {
		csim_iocomp_inst_t *ioi = (csim_iocomp_inst_t *)i;
		ioi->next = board->iocomps;
		board->iocomps = ioi;
	}

	/* call preparation of the instance */
	if(CSIM_DEBUG >=board->level)
		board->log(board, CSIM_INFO, "new instance %s of %s at %08x", name, comp->name, name);
	comp->construct(i, confs);

	/* record the IO registers */
	if(board->mem != NULL)
		csim_record_regs(board, i);
	else if(comp->type == CSIM_CORE) {
		board->mem = csim_core_memory((csim_core_inst_t *)i);
		for(csim_inst_t *ui = board->insts; ui != NULL; ui = ui->next)
			csim_record_regs(board, ui);
	}

	return i;
}


/**
 * Delete the given component.
 * @param inst	Component instance to delete.
 * @ingroup csim
 */
void csim_delete_component(csim_inst_t *inst) {
	csim_board_t *b = inst->board;
	b->log(b, CSIM_INFO, "deleting %s (%s)", inst->name, inst->comp->name);
	inst->comp->destruct(inst);
	free(inst);
}


/**
 * Find a component instance by its name.
 * @param board		Board to look in.
 * @param name		Name of looked component instance.
 * @return			Found instance or NULL.
 */
csim_inst_t *csim_find_instance(csim_board_t *board, const char *name) {
	for(csim_inst_t *i = board->insts; i != NULL; i = i->next)
		if(strcmp(i->name, name) == 0)
			return i;
	return NULL;
}


/**
 * Look for a port matching the name in the component.
 * @param comp		Component to look for port in.
 * @param name		Name of the looked port.
 * @return			Found port or NULL.
 */
csim_port_t*csim_find_port(csim_component_t *comp, const char *name) {
	for(int i = 0; i < comp->port_cnt; i++)
		if(strcmp(comp->ports[i].name, name) == 0)
			return &comp->ports[i];
	return NULL;
}


/**
 * Link two ports.
 * @param inst1	Component instance 1.
 * @param port1	Pin 1.
 * @param inst2	Component instance 2.
 * @param port2	Pin 2.
 * @ingroup csim
 */
void csim_connect(csim_inst_t *inst1, csim_port_t *port1, csim_inst_t *inst2, csim_port_t *port2) {
	assert(inst1->board == inst2->board);
	csim_board_t *b = inst1->board;
	
	/* compute pin index */
	int i1 = port1 - inst1->comp->ports;
	assert(0 <= i1 && i1 < inst1->comp->port_cnt);
	csim_port_inst_t *p1 = &inst1->ports[i1];
	int i2 = port2 - inst2->comp->ports;
	assert(0 <= i2 && i2 < inst2->comp->port_cnt);
	csim_port_inst_t *p2 = &inst2->ports[i2];

	/* already connected? */
	if(p1->link != NULL) {
		if(CSIM_ERROR <= b->level)
			b->log(b, CSIM_ERROR, "%s of %s is already connected!", port1->name, inst1->name);
		return;
	}
	if(p2->link != NULL) {
		if(CSIM_ERROR <= b->level)
			b->log(b, CSIM_ERROR, "%s of %s is already connected!", port2->name, inst2->name);
		return;
	}
	
	/* connect the ports */
	if(CSIM_DEBUG <= b->level)
		b->log(b, CSIM_DEBUG, "connecting %s of %s with %s of %s", port1->name, inst1->name, port2->name, inst2->name);
	p1->link = p2;
	p2->link = p1;
}


/**
 * Disconnect a link.
 * @param inst1	Component instance 1.
 * @param port1	Pin 1.
 * @param inst2	Component instance 2.
 * @param port2	Pin 2.
 * @ingroup csim
 */
void csim_disconnect(csim_inst_t *inst1, csim_port_t *port1, csim_inst_t *inst2, csim_port_t *port2) {
	assert(inst1->board == inst2->board);
	csim_board_t *b = inst1->board;

	/* compute pin index */
	int i1 = port1 - inst1->comp->ports;
	assert(0 <= i1 && i1 < inst1->comp->port_cnt);
	csim_port_inst_t *p1 = &inst1->ports[i1];
	int i2 = port2 - inst2->comp->ports;
	assert(0 <= i2 && i2 < inst2->comp->port_cnt);
	csim_port_inst_t *p2 = &inst2->ports[i2];

	/* disconnect ports */
	if(CSIM_DEBUG <= b->level)
		b->log(b, CSIM_DEBUG, "disconnecting %s of %s with %s of %s", port1->name, inst1->name, port2->name, inst2->name);
	p1->link = NULL;
	p2->link = NULL;
}


/**
 * Stop emitting on the port.
 * @param inst	Instance containing the port.
 * @param port	Port to listen to.
 * @ingroup csim
 */
void csim_mute(csim_inst_t *inst, csim_port_t *port) {
	csim_board_t *b = inst->board;
	assert(port->type == CSIM_ELECTRIC);

	/* compute index */
	int i = port - inst->comp->ports;
	assert(0 <= i && i < inst->comp->port_cnt);
	csim_port_inst_t *pi = &inst->ports[i];

	/* set mute if any */
	if(CSIM_DEBUG <= b->level)
		b->log(b, CSIM_DEBUG, "muting from %s of %s (%d)", port->name, inst->name, b->date);
	
	/* update if needed */
	if(pi->link != NULL) {
		csim_value_t v;
		pi->link->port->update(pi->link, CSIM_NONE, v);
	}
}


/**
 * Send a digital message to a port.
 * @param inst	Instance sending the event.
 * @param port	Pin to send to.
 * @param digit	Digital value to send.
 * @ingroup csim
 */
void csim_send_digital(csim_inst_t *inst, csim_port_t *port, int digit) {
	csim_board_t *b = inst->board;
	assert(port->type == CSIM_DIGITAL);

	/* compute index */
	int i = port - inst->comp->ports;
	assert(0 <= i && i < inst->comp->port_cnt);
	csim_port_inst_t *pi = &inst->ports[i];
	
	/* update the port */
	if(CSIM_DEBUG <= b->level)
		b->log(b, CSIM_DEBUG, "sending digital %d (%d) to %s of %s", digit, b->date, port->name, inst->name);
	
	/* update distant port if any */
	if(pi->link != NULL) {
		csim_value_t v;
		v.digital = digit;
		pi->link->port->update(pi->link, CSIM_DIGITAL, v);
	}
}


/**
 * Record a new event in the event queue.
 * @param board		Board to record event in.
 * @param evt		Event to record.
 * @ingroup csim
 */
void csim_record_event(csim_board_t *board, csim_evt_t *evt) {
	if(CSIM_DEBUG >= board->level)
		board->log(board, CSIM_DEBUG, "record event at %d from %s", evt->date, evt->inst->name);
	
	if(evt->date <= board->date) {
		if(CSIM_DEBUG >= board->level)
			board->log(board, CSIM_DEBUG, "trigger event from %s", evt->inst->name);
		evt->trigger(evt);
		if(evt->period == 0)
			return;
		else
			evt->date += evt->period;
	}
	
	if(board->evts == NULL || evt->date < board->evts->date) {
		evt->next = board->evts;
		board->evts = evt;
	}
	else {
		csim_evt_t *cur = board->evts;
		while(cur->next != NULL && cur->date < evt->date)
			cur = cur->next;
		if(cur->next != NULL)
			cur->next->prev = evt;
		evt->next = cur->next;
		cur->next = evt;
		evt->prev = cur;
	}
}


/**
 * Remove an event from the schedule.
 * @param board		Board to work with.
 * @param evt		Event to cancel.
 * @ingroup csim
 */
void csim_cancel_event(csim_board_t *board, csim_evt_t *evt) {
	if(board->evts->inst == evt->inst && board->evts->trigger == evt->trigger) {
		evt->next->prev = NULL;
		board->evts = evt->next;
		evt->next = NULL;
	}
	else {
		evt->next->prev = evt->prev;
		evt->prev->next = evt->next;
		evt->next = NULL;
		evt->prev = NULL;
	}
}


/**
 * Simulate for the given amount of time.
 * @param board		Board to simulate in.
 * @param time		Time in cycle (cycle duration depends on the board clock).
 * @ingroup csim
 */
void csim_run(csim_board_t *board, csim_time_t time) {
	csim_date_t end = board->date + time;
	while(board->date < end) {
		csim_log(board, CSIM_DEBUG, "next");
		
		while(board->evts != NULL && board->evts->date <= board->date) {
			csim_evt_t *evt = board->evts;
			if(CSIM_DEBUG >= board->level)
				board->log(board, CSIM_DEBUG, "trigger event from %s", evt->inst->name);
			evt->trigger(evt);
			board->evts = evt->next;
			if(board->evts != NULL)
				board->evts->prev = NULL;
			evt->next = NULL;
			if(evt->period != 0) {
				evt->date += evt->period;
				csim_record_event(board, evt);
			}
		}
		
		csim_core_inst_t *core = board->cores;
		while(core != NULL) {
			((csim_core_t *)core->inst.comp)->step(core);
			core = core->next;
		}
		
		board->date++;
	}
}

/**
 * Provides a default implementation for IO get/set_state function pointer.
 * @param inst	IO component instance.
 * @param state	Buffer to store state inside.
 * @param size	Size of state in pairs of uint32_t.
 */
void csim_no_state(csim_iocomp_inst_t *inst, uint32_t *state) {
}


extern csim_component_t *comps[];

/**
 * Find a component by its name, possibly using some mechanism to get access
 * to it.
 * @param name	Component name.
 * @return		Component definition or NULL.
 */
csim_component_t *csim_find_component(const char *name) {
	for(int i = 0; comps[i] != NULL; i++)
		if(strcmp(name, comps[i]->name) == 0)
			return comps[i];
	return NULL;
}

