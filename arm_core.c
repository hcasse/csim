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

#include "csim.h"

#include <arm/api.h>
#include <arm/loader.h>
#include <arm/mem.h>

#define CSIM_PAGE_SIZE	4096
#include "arm_core.h"


// Adresses des interruptions, l'indice dans le tableau est le code d'interruption.
int TAB_INTERRUPT[] = {0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0};

typedef struct arm_core_inst_t {
	csim_core_inst_t inst;
	arm_platform_t *pf;
	arm_memory_t *mem;
	arm_state_t *state;
	arm_sim_t *sim;
} arm_core_inst_t;

//static csim_reg_t regs[] = {};
static csim_port_t ports[] = {};

static void construct(csim_inst_t *inst, csim_confs_t confs) {
	arm_core_inst_t *i = (arm_core_inst_t *)inst;
	i->pf = arm_new_platform();
	i->state = arm_new_state(i->pf);
	i->sim = arm_new_sim(i->state, 0, 0);
	i->mem = arm_get_memory(i->pf, ARM_MAIN_MEMORY);

}

static void destruct(csim_inst_t *inst) {
	arm_core_inst_t *i = (arm_core_inst_t *)inst;
	arm_delete_sim(i->sim);
}

static void reset(csim_inst_t *inst) {
	arm_core_inst_t *i = (arm_core_inst_t *)inst;
	arm_reset_platform(i->pf);
	arm_reset_state(i->state);
}

static void csim_arm_step(csim_core_inst_t *_inst) {
	arm_core_inst_t *inst = (arm_core_inst_t *)_inst;
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

static unsigned inst_size(csim_core_inst_t *_inst) {
	arm_core_inst_t *inst = (arm_core_inst_t *)_inst;
	arm_inst_t *i = arm_next_inst(inst->sim);
	unsigned res = arm_get_inst_size(i);
	arm_free_inst(i);
	return res;
}

static void arm_make_R(csim_inst_t *inst, int num, char *buf, int size) {
	snprintf(buf, size, "R%d", num);
}

static csim_word_t arm_get_R(csim_inst_t *_inst, int num) {
	arm_core_inst_t *inst = (arm_core_inst_t *)_inst;
	return inst->state->GPR[num];
}

static void arm_set_R(csim_inst_t *_inst, int num, csim_word_t val) {
	arm_core_inst_t *inst = (arm_core_inst_t *)_inst;
	inst->state->GPR[num] = val;
}

static csim_word_t arm_get_CPSR(csim_inst_t *_inst, int num) {
	arm_core_inst_t *inst = (arm_core_inst_t *)_inst;
	return inst->state->Ucpsr;
}

static void arm_set_CPSR(csim_inst_t *_inst, int num, csim_word_t val) {
	arm_core_inst_t *inst = (arm_core_inst_t *)_inst;
	inst->state->Ucpsr = val;
}

static void arm_make_CPSR(csim_inst_t *inst, int num, char *buf, int size) {
	strcpy(buf, "CPSR");
}

static void arm_display_R(csim_inst_t *inst_, int num, char *buf, int size) {
	arm_core_inst_t *inst = (arm_core_inst_t *)inst_;
	snprintf(buf, size, "%d", inst->state->GPR[num]);
}

static void arm_display_CPSR(csim_inst_t *inst_, int num, char *buf, int size) {
	arm_core_inst_t *inst = (arm_core_inst_t *)inst_;
	snprintf(buf, size, "%08x", inst->state->Ucpsr);
}

static csim_word_t arm_null_read(csim_inst_t *inst, int num) { return 0; }
static void arm_null_write(csim_inst_t *inst, int num, csim_word_t val) { }


static void arm_install(csim_core_inst_t *_inst, csim_reg_t *reg, csim_addr_t addr) {
	arm_core_inst_t *inst = (arm_core_inst_t *)_inst;
	csim_addr_t pa = addr & ~(CSIM_PAGE_SIZE - 1);
	if(arm_get_callback_data(inst->mem, pa) == NULL)
		arm_set_range_callback(inst->mem, pa, pa+reg->size-1, csim_on_io, _inst->inst.board);

}

static void arm_uninstall(csim_core_inst_t *inst, csim_reg_t *reg, csim_addr_t addr) {
	// uninstall impossible for now
}

static uint8_t arm_load_byte(csim_core_inst_t *_inst, csim_addr_t addr) {
	arm_core_inst_t *inst = (arm_core_inst_t *)_inst;
	return arm_mem_read8(inst->mem, addr);
}

static uint16_t arm_load_half(csim_core_inst_t *_inst, csim_addr_t addr) {
	arm_core_inst_t *inst = (arm_core_inst_t *)_inst;
	return arm_mem_read16(inst->mem, addr);
}

static uint32_t arm_load_word(csim_core_inst_t *_inst, csim_addr_t addr) {
	arm_core_inst_t *inst = (arm_core_inst_t *)_inst;
	return arm_mem_read32(inst->mem, addr);
}

static void arm_store_byte(csim_core_inst_t *_inst, csim_addr_t addr, uint8_t val) {
	arm_core_inst_t *inst = (arm_core_inst_t *)_inst;
	arm_mem_write8(inst->mem, addr, val);
}

static void arm_store_half(csim_core_inst_t *_inst, csim_addr_t addr, uint16_t val) {
	arm_core_inst_t *inst = (arm_core_inst_t *)_inst;
	arm_mem_write16(inst->mem, addr, val);
}

static void arm_store_word(csim_core_inst_t *_inst, csim_addr_t addr, uint32_t val) {
	arm_core_inst_t *inst = (arm_core_inst_t *)_inst;
	arm_mem_write32(inst->mem, addr, val);
}

static csim_reg_t arm_regs[] = {
	{
		"R", 0, 4, 16, 1, CSIM_INTERN, CSIM_INT,
		arm_make_R,
		arm_display_R,
		arm_null_read,
		arm_null_write,
		arm_get_R,
		arm_set_R
	},
	{
		"CPSR", 0, 4, 1, 1, CSIM_INTERN, CSIM_INT,
		arm_make_CPSR,
		arm_display_CPSR,
		arm_null_read,
		arm_null_write,
		arm_get_CPSR,
		arm_set_CPSR
	}
};


/**
 * Component representing an ARM core.
 *
 * Parameters:
 * * mult=INT -- number of instructions executed for each board cycle.
 *
 * @ingroup csim
 */
csim_core_t arm_component = {
	{
		"arm",
		CSIM_CORE,
		1,		// version
		arm_regs, 2,
		ports, 0,
		sizeof(arm_core_inst_t),
		construct,
		destruct,
		reset,
		csim_default_update
	},
	csim_arm_step,
	load,
	pc,
	disasm,
	memory,
	interrupt,
	inst_size,
	arm_install,
	arm_uninstall,
	arm_load_byte,
	arm_load_half,
	arm_load_word,
	arm_store_byte,
	arm_store_half,
	arm_store_word
};
