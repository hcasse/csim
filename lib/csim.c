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
#include <ctype.h>
#include <memory.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define _GNU_SOURCE
#include <dlfcn.h>

#include "csim.h"
#include "arm_core.h"
#include "button.h"
#include "led.h"
#include "sseg.h"

#define CSIM_DEFAULT_CLOCK	1000
#define CSIM_COMP_MAX		256

/**
 * Defines the available components.
 * @ingroup csim
 */
csim_component_t *csim_comps[CSIM_COMP_MAX];

/**
 * Count the number of entries used in @ref csim_comps.
 */
int csim_comp_cnt = 0;


/**
 * Defines loaded libraries (array ended by 0).
 * @ingroup csim
 */
static void *csim_libs[32] = { 0 };

/**
 * List of directories to find plug-in.
 * @ingroup csim
 */
static const char *csim_path[32];


/**
 * Count of entries used in @ref csim_path.
 * @ingroup csim
 */
static int csim_path_cnt = 0;


///
static csim_iostate_t iostate_end = {NULL, { 0, 0, 0 } };

/**
 * Free data allocated for a configuration.
 * @param confs		Configuration to save.
 */
static void csim_free_confs(csim_confs_t confs) {
	free((char **)confs);
}

/**
 * Allocate a new version of configuration.
 * @param confs		Configurations to copy.
 * @return			Copied configuration to free with @ref csim_free_confs().
 */
static csim_confs_t csim_copy_confs(csim_confs_t confs) {

	// compute size and count
	int cnt = 0;
	int size = 0;
	while(confs[cnt] != NULL) {
		size += strlen(confs[cnt]) + 1;
		cnt++;
	}

	// allocate and prepare pointer
	char *data = (char *)malloc(sizeof(char *) * (cnt + 1) + size);
	csim_confs_t copy = (csim_confs_t)data;
	char *buf = data + sizeof(char *) * (cnt + 1);

	// copy the content
	for(int i = 0; i < cnt; i++) {
		copy[i] = buf;
		strcpy(buf, confs[i]);
		buf += strlen(confs[i]) + 1;
	}
	copy[cnt] = NULL;

	return copy;
}

/**
 * Parse the provided string for an integer supporint decimal, 0x hedaxdecimal
 * and 0b binary.
 */
static uint32_t parse_int(const char *str) {

	// remove spaces
	while(isblank(*str))
		str++;

	// compute base
	int base = 10;
	if(str[0] == '0')
		switch(str[1]) {
		case 'x': case 'X':
			base = 16;
			str += 2;
			break;
		case 'b': case 'B':
			base = 2;
			str += 2;
		}

	// perform conversion
	return strtoul(str, NULL, base);
}


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
 * Initialize the CSIM system.
 * @ingroup csim
 */
static void csim_init() {
	static int init = 0;
	if(!init) {

		// initialize list of default components
		init = 1;
		csim_register_component(&led_component.comp);
		csim_register_component(&button_component.comp);
		csim_register_component(&arm_component.comp);
		csim_register_component(&sseg_component.comp);

		// initialize default path
		char *path = getenv("CSIMPATH");
		if(path == NULL) {
			csim_add_path("");
			csim_add_path(".");
		}
		else {
			char *p = strchr(path, ':');
			while(p != NULL) {
				*p = '\0';
				csim_add_path(path);
				path = p + 1;
				p = strchr(path, ':');
			}
			csim_add_path(path);
		}
	}
}


/**
 * Add a path to the paths looked for a plug-in.
 * @param path	Added path.
 */
void csim_add_path(const char *path) {
	csim_init();
	if(csim_path_cnt == 32) {
		fprintf(stderr, "ERROR: too many added plug-in paths!\n");
		abort();
	}
	csim_path[csim_path_cnt++] = path;
}


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

	if(board->level > level)
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
 * Perform a read access to an IO register. Log an error and returns -1 if
 * there no corresponding IO register.
 * @param board		Current board.
 * @param addr		Accessed address.
 * @param size		Size of access (in bytes).
 * @return			Read word.
 */
csim_word_t csim_read_io(csim_board_t *board, csim_addr_t addr, int size) {
	int h = CSIM_IO_HASH(addr);
	for(csim_io_t *p = board->ios[h]; p != NULL; p = p->next)
		if(p->addr == addr) {
			int i = (addr - p->inst->base - p->reg->offset) / p->reg->stride;
			if(size != p->reg->size)
				board->log(board, CSIM_ERROR, "bad IO access at %08x: size=%d and should be %d", addr, size, p->reg->size);
			return p->reg->read(p->inst, i);
		}
	board->log(board, CSIM_ERROR, "bad IO access, unknown address at %08x", addr, size);
	return -1;
}


/**
 * Perform a write access to an IO register. Log an error if there is no
 * corresponding IO register.
 * @param board		Current board.
 * @param addr		Accessed address.
 * @param size		Size of access (in bytes).
 */
void csim_write_io(csim_board_t *board, csim_addr_t addr, int size, csim_word_t word) {
	int h = CSIM_IO_HASH(addr);
	for(csim_io_t *p = board->ios[h]; p != NULL; p = p->next)
		if(p->addr == addr) {
			int i = (addr - p->inst->base - p->reg->offset) / p->reg->stride;
			if(size != p->reg->size)
				board->log(board, CSIM_ERROR, "bad IO access at %08x: size=%d and should be %d", addr, size, p->reg->size);
			p->reg->write(p->inst, i, word);
			return;
		}
	board->log(board, CSIM_ERROR, "bad IO access, unknown address at %08x", addr, size);
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
void csim_on_io(csim_addr_t addr, int size, void *data, int access, void *cdata) {
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
 * Install a memory a component register.
 * @param board		Current board.
 * @param reg		Register.
 * @param addr		Address.
 * @ingroup csim
 */
static void csim_io_install(
	csim_board_t *board,
	csim_reg_t *reg,
	csim_addr_t addr
) {
	/*csim_addr_t pa = addr & ~(CSIM_PAGE_SIZE - 1);
	if(csim_get_callback_data(board->mem, pa) == NULL)
		csim_set_range_callback(board->mem, pa, pa + reg->size - 1, csim_on_io, board);*/
	csim_core_inst_t *core = board->cores;
	assert(core);
	csim_core(core)->install(core, reg, addr);
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
		csim_io_install(board, reg, a);
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
 * Build a new board.
 * @param name	Board name.
 * @return		Built board (or null if allocation fails).
 * @ingroup csim
 */
csim_board_t *csim_new_board(const char *name) {
	const char *conf[] = { "name", name, NULL };
	return csim_new_board_ext(conf);
}


/**
 * Build a new board wit configuration.
 *
 * Supported configuration:
 * * clock=INT -- board clock (default 1000Hz).
 * * name=STRING -- name of the board
 * * log=[0-5] -- select log level
 *
 * @param conf	Configuration.
 * @return		Built board (or null if allocation fails).
 * @ingroup csim
 */
csim_board_t *csim_new_board_ext(csim_confs_t conf) {
	csim_init();

	// duplicate configuration
	csim_confs_t my_confs = csim_copy_confs(conf);

	/* build the board */
	csim_board_t *board = (csim_board_t *)malloc(sizeof(csim_board_t));
	if(board == NULL)
		return NULL;
	board->name = "no name";
	board->inst_cap = 32;
	board->inst_cnt = 0;
	board->insts = (csim_inst_t **)malloc(sizeof(csim_inst_t *) * board->inst_cap);
	board->cores = NULL;
	board->iocomps = NULL;
	board->date = 0;
	board->evts = NULL;
	board->level = CSIM_INFO;
	board->pending = NULL;
	board->iostates_head = &iostate_end;
	board->iostates_count = 0;
	board->confs = my_confs;
	board->log = csim_log;
	memset(board->ios, 0, sizeof(csim_io_t *) * CSIM_IO_SIZE);

	/* scan the configuration */
	csim_clock_t clock = CSIM_DEFAULT_CLOCK;
	for(int i = 0; conf[i]; i += 2) {
		if(strcmp(conf[i], "clock") == 0) {
			int err;
			clock = csim_parse_uint(conf[i + 1], &err);
			if(err) {
				csim_log(board, CSIM_ERROR, "bad clock expression '%s'", conf[i+1]);
				clock = CSIM_DEFAULT_CLOCK;
			}
			else
				board->log(board, CSIM_DEBUG, "clock = %ld", clock);
		}
		else if(strcmp(conf[i], "name") == 0) {
			board->name = my_confs[i + 1];
			board->log(board, CSIM_DEBUG, "name = %s", board->name);
		}
		else if(strcmp(conf[i], "log") == 0) {
			board->level = strtoul(conf[i+1], NULL, 10);
			board->log(board, CSIM_DEBUG, "log level = %d", board->level);
		}
	}
	board->clock = clock;

	/* return board */
	return board;
}


/**
 * Delete the given board.
 * @param board		Board to delete.
 * @ingroup csim
 */
void csim_delete_board(csim_board_t *board) {

	for(int i = 0; i < board->inst_cnt; i++)
		csim_delete_component(board->insts[i]);

	for(int i = 0; i < CSIM_IO_SIZE; i++) {
		csim_io_t *p = board->ios[i];
		while(p != NULL) {
			csim_io_t *q = p->next;
			free(p);
			p = q;
		}
	}

	board->log(board, CSIM_DEBUG, "deleting board %s", board->name);
	csim_free_confs(board->confs);
	free(board->insts);
	free(board);
}


/**
 * Reset the state of the board keeping the components.
 * @param board	Board to reset.
 * @ingroup csim
 */
void csim_reset_board(csim_board_t *board) {

	/* reset all component instances */
	for(int i = 0; i < board->inst_cnt; i++) {
		board->insts[i]->comp->reset(board->insts[i]);
		for(csim_init_t *init = board->insts[i]->inits; init; init = init->next)
			for(int j = 0; j < init->reg->count; j++)
				init->reg->set(board->insts[i], j, init->val);
	}

	/* reset time */
	board->date = 0;
	board->evts = NULL;

	/* re-install callbacks */
	for(int i = 0; i < CSIM_IO_SIZE; i++)
		for(csim_io_t *p = board->ios[i]; p != NULL; p = p->next)
			csim_io_install(board, p->reg, p->addr);
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
	char base_str[16];
	sprintf(base_str, "%08x", base);
	const char *confs[] = { "name", name, "base", base_str, NULL };
	return csim_new_component_ext(board, comp, confs);
}

/**
 * Record the registers of the component in the address space.
 * @param board		Current board.
 * @param inst		Component instance.
 */
static void csim_record_regs(csim_board_t *board, csim_inst_t *inst) {
	csim_component_t *comp = inst->comp;
	for(int j = 0; j < comp->reg_cnt; j++)
		if((comp->regs[j].flags & CSIM_INTERN) == 0)
		csim_io_add(&comp->regs[j], inst);
}

/**
 * Default update function that does nothing.
 * @param inst	Component instance to update.
 */
void csim_default_update(csim_inst_t *inst) {
}


/**
 * Build a new instance of the given component and add it to the board.
 *
 * confs is a null-terminated array of strings organized by pairs which
 * first member is the entry name and the second the entry value.
 *
 * Configuration:
 * * name=STRING -- component name
 * * base=INT -- base address of the component
 *
 * @param board	Board to add to.
 * @param comp	Component to build an instance for.
 * @param confs	Configurations.
 * @return		Built instance.
 * @ingroup csim
 */
csim_inst_t *csim_new_component_ext(csim_board_t *board, csim_component_t *comp, csim_confs_t confs) {
	csim_confs_t my_confs = csim_copy_confs(confs);
	csim_init_t *inits = NULL;

	/* parse configuration */
	const char *name = "no name";
	csim_addr_t base = 0;
	for(int i = 0; confs[i]; i += 2)
		if(strcmp(confs[i], "name") == 0) {
			name = my_confs[i + 1];
			board->log(board, CSIM_DEBUG, "name=%s", name);
		}
		else if(strcmp(confs[i], "base") == 0) {
			base = strtoul(confs[i+1], NULL, 16);
			board->log(board, CSIM_DEBUG, "base=%08x", base);
		}
		else {
			for(int j = 0; j < comp->reg_cnt; j++) {
				if(strcmp(confs[i], comp->regs[j].name) == 0) {
					csim_init_t *init = (csim_init_t *)malloc(sizeof(csim_init_t));
					init->next = inits;
					inits = init;
					init->reg = &comp->regs[j];
					init->val = parse_int(confs[i+1]);
					if(board->level <= CSIM_DEBUG)
						board->log(board, CSIM_DEBUG, "init %s = %08x (%s)\n", confs[i], init->val, confs[i+1]);
					break;
				}
			}
		}

	/* build the instance */
	uint8_t *p = (uint8_t *)malloc(comp->size + comp->port_cnt * sizeof(csim_port_inst_t));
	csim_inst_t *inst = (csim_inst_t *)p;
	inst->next_pending = NULL;
	inst->flags = 0;
	inst->comp = comp;
	inst->name = name;
	inst->base = base;
	inst->board = board;
	inst->ports = (csim_port_inst_t *)(p + comp->size);
	for(int j = 0; j < comp->port_cnt; j++) {
		inst->ports[j].port = &comp->ports[j];
		inst->ports[j].inst = inst;
		inst->ports[j].link = NULL;
	}
	inst->id = board->inst_cnt;
	inst->confs = my_confs;
	inst->inits = inits;

	/* determine number in the component instances */
	inst->number = 0;
	for(int i = 0; i < board->inst_cnt; i++)
		if(board->insts[i]->comp == comp)
			inst->number++;

	/* link to the board */
	if(board->inst_cnt == board->inst_cap) {
		board->inst_cap *= 2;
		board->insts = (csim_inst_t **)realloc(board->insts, sizeof(csim_inst_t *) * board->inst_cap);
	}
	board->insts[board->inst_cnt] = inst;
	board->inst_cnt++;

	/* if core, record it in core list */
	if(comp->type == CSIM_CORE) {
		csim_core_inst_t *ci = (csim_core_inst_t *)inst;
		ci->next = board->cores;
		board->cores = ci;
	}

	/* If IO component, record it in the IO list. */
	if(comp->type == CSIM_IO) {
		csim_iocomp_inst_t *ioi = (csim_iocomp_inst_t *)inst;
		ioi->next = board->iocomps;
		board->iocomps = ioi;
	}

	/* call preparation of the instance */
	if(CSIM_DEBUG >= board->level)
		board->log(board, CSIM_INFO, "new instance %s of %s at %08x", name, comp->name, name);
	comp->construct(inst, confs);
	for(csim_init_t *init = inits; init; init = init->next)
		for(int j = 0; j < init->reg->count; j++)
			init->reg->set(inst, j, init->val);

	/* record the IO registers */
	if(board->cores != NULL)
		csim_record_regs(board, inst);
	else if(comp->type == CSIM_CORE)
		for(int i = 0; i < board->inst_cnt; i++)
			csim_record_regs(board, board->insts[i]);

	return inst;
}


/**
 * Delete the given component.
 * @param inst	Component instance to delete.
 * @ingroup csim
 */
void csim_delete_component(csim_inst_t *inst) {
	csim_board_t *b = inst->board;
	b->log(b, CSIM_DEBUG, "deleting %s (%s)", inst->name, inst->comp->name);
	csim_free_confs(inst->confs);
	inst->comp->destruct(inst);
	free(inst);
}


/**
 * Record a component ready to be updated.
 * @param inst		Instance to wake up.
 */
void csim_wakeup(csim_inst_t *inst) {
	if(!(inst->flags & CSIM_PENDING)) {
		csim_board_t *board = inst->board;
		inst->flags |= CSIM_PENDING;
		inst->next_pending = board->pending;
		board->pending = inst;
	}
}


/**
 * Find a component instance by its name.
 * @param board		Board to look in.
 * @param name		Name of looked component instance.
 * @return			Found instance or NULL.
 */
csim_inst_t *csim_find_instance(csim_board_t *board, const char *name) {
	for(int i = 0; i < board->inst_cnt; i++)
		if(strcmp(board->insts[i]->name, name) == 0)
			return board->insts[i];
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

	// get port instance
	csim_port_inst_t *pi = csim_port_inst(inst, port);

	/* log operation */
	if(CSIM_DEBUG <= b->level)
		b->log(b, CSIM_DEBUG, "sending digital %d (%d) to %s of %s", digit, b->date, port->name, inst->name);

	/* update distant port if any if required */
	if(pi->link != NULL) {
		csim_value_t value;
		value.digital = digit;
		pi->link->port->update(pi->link, CSIM_DIGITAL, value);
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
 * Consume all events for the current date.
 * @param board		Current board.
 */
static void consume_events(csim_board_t *board) {
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
}


/**
 * Update the components requiring it.
 * @param board		Current board.
 */
static void update_components(csim_board_t *board) {
	while(board->pending) {
		csim_inst_t *inst = board->pending;
		board->pending = inst->next_pending;
		inst->flags &= ~CSIM_PENDING;
		inst->comp->update(inst);
	}
}


/**
 * Simulate for the given amount of time.
 * @param board		Board to simulate in.
 * @param time		Time in cycle (cycle duration depends on the board clock).
 * @return			0 if the execution is successful, 1 if the execution encountered a break.
 * @ingroup csim
 */
int csim_run(csim_board_t *board, csim_time_t time) {
	csim_date_t end = board->date + time;
	int at_break = 0;
	while(board->date < end && !at_break) {
		csim_log(board, CSIM_DEBUG, "next");
		consume_events(board);
		for(csim_core_inst_t *core = board->cores; core; core = core->next) {
			int res = ((csim_core_t *)core->inst.comp)->step(core);
			at_break |= res;
		}
		update_components(board);
		board->date++;
	}
	return at_break;
}

/**
 * Execute the current instruction and increase by 1 cycle.
 * @param board		Current board.
 */
void csim_step(csim_board_t *board) {
	csim_log(board, CSIM_DEBUG, "step");
	consume_events(board);
	for(csim_core_inst_t *core = board->cores; core; core = core->next)
		((csim_core_t *)core->inst.comp)->step(core);
	update_components(board);
	board->date++;
}

/**
 * Provides a default implementation for IO get/set_state function pointer.
 * @param inst	IO component instance.
 * @param state	Buffer to store state inside.
 * @param size	Size of state in pairs of uint32_t.
 * @ingroup csim
 */
void csim_no_state(csim_iocomp_inst_t *inst, uint32_t *state) {
}


/**
 * Lookup for a component in the current list of components.
 * @param name	Name of looked component.
 * @return		Found component or NULL.
 */
static csim_component_t *csim_lookup_component(const char *name) {
	for(int i = 0; i < csim_comp_cnt; i++)
		if(strcmp(name, csim_comps[i]->name) == 0)
			return csim_comps[i];
	return NULL;
}


/**
 * Find a component by its name, possibly using some mechanism to get access
 * to it.
 * @param name	Component name.
 * @return		Component definition or NULL.
 * @ingroup csim
 */
csim_component_t *csim_find_component(const char *name) {

	// look in the list
	csim_component_t *comp = csim_lookup_component(name);
	if(comp)
		return comp;

	// not found: try to find a plug-in
	const char *p = strchr(name, '/');
	if(p == NULL)
		return NULL;

	// build the plug-in name
	char lib[32];
	strncpy(lib, name, p - name);
	lib[p - name] = '\0';
	char libname[256];
	snprintf(libname, 256, "lib%s.so", lib);

	// test for each entry in the handle
	void *handle = NULL;
	for(int i = 0; i < csim_path_cnt && handle == NULL; i++) {

		// build the path
		char path[512];
		if(*csim_path[i] == '\0')
			strcpy(path, libname);
		else
			snprintf(path, 512, "%s/%s", csim_path[i], libname);

		// load it
		handle = dlopen(path, RTLD_LAZY | RTLD_GLOBAL);
	}
	if(handle == NULL) {
		fprintf(stderr, "ERROR: cannot find plugin %s\n", libname);
		return NULL;
	}

	// already opened?
	for(int i = 0; csim_libs[i]; i++)
		if(csim_libs[i] == handle)
			return NULL;

	// get the function
	char fun_name[256];
	snprintf(fun_name, 256, "%s_get_components", lib);
	void *sym = dlsym(handle, fun_name);
	if(!sym) {
		fprintf(stderr, "ERROR: cannot find symbol %s in plugin %s\n", fun_name, libname);
		return NULL;
	}
	typedef csim_component_t **(*fun_t)();
	fun_t fun = (fun_t)sym;

	// register the components
	csim_component_t **comps = fun();
	while(*comps) {
		csim_register_component(*comps);
		comps++;
	}

	// record the library
	int i;
	for(i = 0; csim_libs[i]; i++);
	csim_libs[i] = handle;
	csim_libs[i+1] = NULL;

	// look back for the component
	return csim_lookup_component(name);
}


/**
 * Register a component to be then retrieved with @ref csim_find_component().
 * @param comp		Component to register.
 */
void csim_register_component(csim_component_t *comp) {
	assert(csim_comp_cnt < CSIM_COMP_MAX);
	comp->index = csim_comp_cnt;
	csim_comps[csim_comp_cnt++] = comp;
}


/**
 * Read a byte in memory.
 * @param board		Current board.
 * @param addr		Address to read byte from.
 * @return			Read byte.
 * @ingroup csim
 */
uint8_t csim_byte_at(csim_board_t *board, csim_addr_t addr) {
	csim_core_inst_t *core = board->cores;
	assert(core);
	return csim_core(core)->load_byte(core, addr);
}

/**
 * Read an half-word in memory.
 * @param board		Current board.
 * @param addr		Address to read half-word from.
 * @return			Read half-word.
 * @ingroup csim
 */
uint16_t csim_half_at(csim_board_t *board, csim_addr_t addr) {
	csim_core_inst_t *core = board->cores;
	assert(core);
	return csim_core(core)->load_half(core, addr);
}

/**
 * Read a word in memory.
 * @param board		Current board.
 * @param addr		Address to read word from.
 * @return			Read word.
 * @ingroup csim
 */
uint32_t csim_word_at(csim_board_t *board, csim_addr_t addr) {
	csim_core_inst_t *core = board->cores;
	assert(core);
	return csim_core(core)->load_word(core, addr);
}

/**
 * Read a long word in memory.
 * @param board		Current board.
 * @param addr		Address to read long word from.
 * @return			Read long word.
 * @ingroup csim
 */
uint64_t csim_long_at(csim_board_t *board, csim_addr_t addr) {
	assert(0 && "unsupported");
}


/**
 * @typedef csim_confs_t;
 * Array of strings to pass configuration to a component. The entries are grouped
 * by pairs (key, value). The end is marked with a NULL key.
 * @ingroup csim
 */


/**
 * Parse a string as an unsigned integer supportin decimal form and
 * "0x"-prefixed hexadecimal.
 * @param str	String to convert.
 * @param err	If not NULL, gets 0 for success or not 0 for failure.
 * @ingroup csim
 */
uint32_t csim_parse_uint(const char *str, int *err) {
	uint32_t val;
	char *end;
	if(str[0] == '0' && (str[1] == 'x' || str[1] == 'X'))
		val = strtoul(str+2, &end, 16);
	else
		val = strtoul(str, &end, 10);
	if(err)
		*err = *end != '\0';
	return val;
}


/**
 * Default function for port update to wakeup the component.
 * Just call @ref csim_wakeup() for the component owning the port.
 */
void csim_port_wakeup(csim_port_inst_t *inst, csim_value_type_t type, csim_value_t val) {
	csim_wakeup(inst->inst);
}


/**
 * Default function to wake up the component when an IO register is written.
 */
void csim_write_wakeup(csim_inst_t *inst, int num, csim_word_t val) {
	csim_wakeup(inst);
}


/**
 * Record the latest version of the IO state.
 * @param inst		Instance of component recording the state.
 * @param state		Recorded IO state.
 */
void csim_record_iostate(csim_inst_t *inst, csim_iostate_t *state) {
	if(state->next)
		return;
	csim_board_t *board = inst->board;
	board->iostates_count++;
	state->next = board->iostates_head;
	board->iostates_head = state;
}


/**
 * Record all IO states for processing.
 * @param board		Current board.
 * @param infos		Buffer to store IO state information in
 * 					(must be of size board->iostates_count).
 */
void csim_flush_iostates(csim_board_t *board, csim_ioinfo_t infos[]) {
	int i = 0;
	for(csim_iostate_t *state = board->iostates_head, *next; state != &iostate_end; state = next) {
		next = state->next;
		state->next = NULL;
		infos[i++] = state->info;
	}
	board->iostates_head = &iostate_end;
	board->iostates_count = 0;
}


/**
 * Perform an input on the IO component identified in the info.
 * @param board		Current board.
 * @param
 */
void csim_do_input(csim_board_t *board, csim_ioinfo_t *info) {
	assert(info->id < board->inst_cnt);
	csim_iocomp_inst_t *inst = (csim_iocomp_inst_t *)board->insts[info->id];
	assert(inst->inst.comp->type == CSIM_IO);
	((csim_iocomp_t *)inst->inst.comp)->change(inst, info);
}


/**
 * Default implementation for the change function of an IO component.
 * @param inst	Instance supporting the change.
 * @param info	Change information.
 */
void csim_default_change(csim_iocomp_inst_t *inst, csim_ioinfo_t *info) {
	inst->inst.board->log(inst->inst.board, CSIM_ERROR, "%d:%s: unsupported change call with %d:%d",
		inst->inst.id, inst->inst.name, info->ress, info->state);
}
