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

#include <string.h>

#define CSIM_INSIDE
#include "mem.h"
#include "csim.h"

#include <arm/api.h>
#include <arm/loader.h>

#define SIM_SLICE	10
#include "arm_core.h"


// Adresses des interruptions, l'indice dans le tableau est le code d'interruption.
int TAB_INTERRUPT[] = {0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0};

typedef struct {
	csim_core_inst_t inst;
	arm_platform_t *pf;
	arm_state_t *state;
	arm_sim_t *sim;
} arm_core_inst_t;

static csim_reg_t regs[] = {};
static csim_port_t ports[] = {};

static void construct(csim_inst_t *inst, csim_confs_t confs) {
	arm_core_inst_t *i = (arm_core_inst_t *)inst;
	i->pf = arm_new_platform();
	i->state = arm_new_state(i->pf);
	i->sim = arm_new_sim(i->state, 0, 0);
}

static void destruct(csim_inst_t *inst) {
	arm_core_inst_t *i = (arm_core_inst_t *)inst;
	arm_delete_sim(i->sim);
}

static void reset(csim_inst_t *inst) {
}

static void step(csim_core_inst_t *_inst) {
	arm_core_inst_t *inst = (arm_core_inst_t *)_inst;
	for(int i = 0; i < SIM_SLICE; i++)
		arm_step(inst->sim);
}

static int load(csim_core_inst_t *_inst, const char *path) {
	arm_core_inst_t *inst = (arm_core_inst_t *)_inst;

	/* load the file */
	arm_loader_t *loader = arm_loader_open(path);
	if(loader == NULL)
		return 1;
	arm_load(inst->pf, loader);

	/* look for _start and _exit */
	for(int i = 0; i < arm_loader_count_syms(loader); i++) {
		arm_loader_sym_t sym;
		arm_loader_sym(loader, i, &sym);
		if(strcmp(sym.name, "_start") == 0)
			arm_set_next_address(inst->sim, sym.value);
		else if(strcmp(sym.name, "_exit") == 0)
			inst->sim->addr_exit = sym.value;
	}

	/* cleanup loader */
	arm_loader_close(loader);
	return 0;
}

csim_addr_t pc(csim_core_inst_t *_inst) {
	arm_core_inst_t *inst = (arm_core_inst_t *)_inst;
	return arm_next_addr(inst->sim);
}

void disasm(csim_core_inst_t *_inst, csim_addr_t addr, char buf[]) {
	arm_core_inst_t *inst = (arm_core_inst_t *)_inst;
	arm_inst_t *i = arm_next_inst(inst->sim);
	arm_disasm(buf, i);
	arm_free_inst(i);
}

void *memory(csim_core_inst_t *_inst) {
	arm_core_inst_t *inst = (arm_core_inst_t *)_inst;
	return arm_get_memory(inst->pf, ARM_MAIN_MEMORY);
}

void interrupt(csim_core_inst_t *_inst,int codeInterrupt) {
	arm_core_inst_t *inst = (arm_core_inst_t *)_inst;
	arm_set_next_address(inst->sim,TAB_INTERRUPT[codeInterrupt]);
	fprintf(stderr,"Interruption numéro : %d\nNouveau pc = %d\n",codeInterrupt,pc(_inst));
}

csim_core_t arm_component = {
	{
		"arm",
		CSIM_CORE,
		1,		// version
		regs, 0,
		ports, 0,
		sizeof(arm_core_inst_t),
		construct,
		destruct,
		reset
	},
	0,	// clock
	step,
	load,
	pc,
	disasm,
	memory,
	interrupt
};
