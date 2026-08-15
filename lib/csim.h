/*
 * GLISS Component simulator main header
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
#ifndef CSIM_H
#define CSIM_H

#include <stdio.h>
#include <stdint.h>

#define CSIM_READ	0x0001
#define CSIM_WRITE	0x0002

typedef uint32_t csim_addr_t;
typedef uint32_t csim_word_t;
typedef uint32_t csim_size_t;
typedef uint64_t csim_date_t;
typedef uint64_t csim_time_t;
typedef uint64_t csim_clock_t;

typedef enum csim_rtype_t {
	CSIM_NORTYPE = 0,
	CSIM_BITS,
	CSIM_INT,
	CSIM_ADDR,
	CSIM_FLOAT32,
	CSIM_FLOAT64
} csim_rtype_t;

typedef uint8_t csim_port_type_t;
#define CSIM_ELECTRIC	0

typedef uint8_t csim_value_type_t;
#define CSIM_NONE		0
#define	CSIM_DIGITAL	1
#define CSIM_ANALOG		2
#define CSIM_CLOCK		3
#define CSIM_SERIAL		4

typedef enum csim_ctype_t {
	CSIM_NOCTYPE = 0,
	CSIM_SIMPLE = 1,
	CSIM_CORE = 2,
	CSIM_IO = 3
} csim_ctype_t;

typedef enum csim_level_t: uint8_t {
	CSIM_NOLOG = 0,
	CSIM_DEBUG = 1,
	CSIM_INFO = 2,
	CSIM_WARN = 3,
	CSIM_ERROR = 4,
	CSIM_FATAL = 5
} csim_level_t;

#define CSIM_PENDING	0x0001

#define CSIM_STATE_STOP	0

typedef uint32_t csim_reg_flags_t;
#define CSIM_INTERN	0x0001

typedef struct csim_component_t csim_component_t;
typedef struct csim_reg_t csim_reg_t;
typedef struct csim_port_t csim_port_t;
typedef struct csim_evt_t csim_evt_t;
typedef struct csim_board_t csim_board_t;
typedef struct csim_core_t csim_core_t;
typedef struct csim_core_inst_t csim_core_inst_t;
#define CSIM_MEM_READ	0
#define CSIM_MEM_WRITE	1
typedef void (*csim_callback_t)(csim_addr_t addr, int size, void *data, int type_access, void *cdata);
typedef struct csim_inst_t csim_inst_t;
typedef struct csim_iocomp_t csim_iocomp_t;
typedef struct csim_iocomp_inst_t csim_iocomp_inst_t;
typedef struct csim_port_inst_t csim_port_inst_t;
typedef struct csim_init_t csim_init_t;
typedef union csim_value_t csim_value_t;
typedef struct csim_iostate_t csim_iostate_t;
typedef struct csim_ioinfo_t csim_ioinfo_t;

struct csim_reg_t {
	const char *name;
	uint32_t offset;
	uint32_t size;
	uint32_t count;
	uint32_t stride;
	csim_reg_flags_t flags;
	csim_rtype_t type;
	void (*make_name)(csim_inst_t *inst, int num, char *buf, int size);
	void (*display)(csim_inst_t *inst, int num, char *buf, int size);
	csim_word_t (*read)(csim_inst_t *inst, int num);
	void (*write)(csim_inst_t *inst, int num, csim_word_t val);
	csim_word_t (*get)(csim_inst_t *inst, int num);
	void (*set)(csim_inst_t *inst, int num, csim_word_t val);
};

union csim_value_t {
	int digital;
	float analog;
	uint32_t clock;
	char serial;
};

/* ports */

struct csim_port_t {
	const char *name;
	csim_port_type_t type;
	void (*update)(csim_port_inst_t *inst, csim_value_type_t type, csim_value_t val);
};


struct csim_port_inst_t {
	csim_port_t *port;
	csim_inst_t *inst;
	csim_port_inst_t *link;
};


/* events */

struct csim_evt_t {
	struct csim_evt_t *next, *prev;
	csim_date_t date;
	uint32_t period;
	csim_inst_t *inst;
	void (*trigger)(csim_evt_t *evt);
};


/* component */

typedef const char **csim_confs_t;

struct csim_init_t {
	csim_init_t *next;
	csim_reg_t *reg;
	uint32_t val;
};

struct csim_component_t {
	int index;
	const char *name;
	csim_ctype_t type;
	uint32_t version;
	csim_reg_t *regs;
	int reg_cnt;
	csim_port_t *ports;
	int port_cnt;
	csim_size_t size;
	void (*construct)(csim_inst_t *inst, csim_confs_t confs);
	void (*destruct)(csim_inst_t *inst);
	void (*reset)(csim_inst_t *inst);
	void (*update)(csim_inst_t *inst);
};

struct csim_inst_t {
	struct csim_inst_t *next;
	struct csim_inst_t *next_core;
	struct csim_inst_t *next_pending;
	struct csim_component_t *comp;
	csim_addr_t base;
	const char *name;
	uint16_t number;
	uint16_t flags;
	uint32_t id;
	csim_board_t *board;
	csim_port_inst_t *ports;
	csim_confs_t confs;
	csim_init_t *inits;
};


/* core component */
struct csim_core_t {
	csim_component_t comp;
	int (*step)(csim_core_inst_t *inst);
	int (*load)(csim_core_inst_t *inst, const char *path);
	csim_addr_t (*pc)(csim_core_inst_t *inst);
	void (*disasm)(csim_core_inst_t *inst, csim_addr_t addr, char buf[]);
	void *(*memory)(csim_core_inst_t *inst);
	void (*interrupt)(csim_core_inst_t *_inst, int codeInterrupt);
	unsigned (*inst_size)(csim_core_inst_t *inst);
	void (*install)(csim_core_inst_t *inst, csim_reg_t *reg, csim_addr_t addr);
	void (*uninstall)(csim_core_inst_t *inst, csim_reg_t *reg, csim_addr_t addr);
	uint8_t (*load_byte)(csim_core_inst_t *inst, csim_addr_t addr);
	uint16_t (*load_half)(csim_core_inst_t *inst, csim_addr_t addr);
	uint32_t (*load_word)(csim_core_inst_t *inst, csim_addr_t addr);
	void (*store_byte)(csim_core_inst_t *inst, csim_addr_t addr, uint8_t val);
	void (*store_half)(csim_core_inst_t *inst, csim_addr_t addr, uint16_t val);
	void (*store_word)(csim_core_inst_t *inst, csim_addr_t addr, uint32_t val);
	void (*set_break)(csim_core_inst_t *inst, csim_addr_t addr);
	void (*clear_break)(csim_core_inst_t *inst, csim_addr_t addr);
};

struct csim_core_inst_t {
	csim_inst_t inst;
	struct csim_core_inst_t *next;
};


/* I/O component */
struct csim_iocomp_t {
	csim_component_t comp;
	int (*display)(char *buf, csim_iocomp_inst_t *inst);
	void (*on_key)(char key, csim_iocomp_inst_t *inst);
	void (*get_state)(csim_iocomp_inst_t *inst, uint32_t *state);
	void (*set_state)(csim_iocomp_inst_t *inst, uint32_t *state);
	void (*change)(csim_iocomp_inst_t *inst, csim_ioinfo_t *state);
};

struct csim_iocomp_inst_t {
	csim_inst_t inst;
	struct csim_iocomp_inst_t *next;
};

struct csim_ioinfo_t {
	uint32_t id;
	uint32_t ress;
	uint32_t state;
};

struct csim_iostate_t {
	csim_iostate_t *next;
	csim_ioinfo_t info;
};

/* board */
#ifndef CSIM_IO_SHIFT
#	define CSIM_IO_SHIFT	2
#endif
#ifndef CSIM_IO_BITS
#	define CSIM_IO_BITS	9
#endif
#define CSIM_IO_SIZE	(1 << CSIM_IO_BITS)

typedef struct csim_io_t {
	struct csim_io_t *next;
	csim_addr_t addr;
	csim_reg_t *reg;
	csim_inst_t *inst;
} csim_io_t;

struct csim_board_t {
	const char *name;
	csim_inst_t **insts;
	uint32_t inst_cnt, inst_cap;
	csim_core_inst_t *cores;
	csim_iocomp_inst_t *iocomps;
	csim_clock_t clock;
	csim_date_t date;
	csim_evt_t *evts;
	csim_level_t level;
	csim_inst_t *pending;
	csim_iostate_t *iostates_head;
	int iostates_count;
	csim_confs_t confs;
	void (*log)(csim_board_t *board, csim_level_t level, const char *msg, ...);
	csim_io_t *ios[CSIM_IO_SIZE];
};

extern csim_component_t *csim_comps[];
extern int csim_comp_cnt;

void csim_add_path(const char *path);
csim_board_t *csim_new_board(const char *name);
csim_board_t *csim_new_board_ext(csim_confs_t conf);
void csim_delete_board(csim_board_t *board);
csim_board_t *csim_load_board(const char *path);
csim_board_t *csim_load_board_ext(const char *path, csim_level_t level);
void csim_reset_board(csim_board_t *board);

csim_component_t *csim_find_component(const char *name);
void csim_register_component(csim_component_t *comp);

csim_inst_t *csim_new_component(csim_board_t *board, csim_component_t *comp, const char *name, csim_addr_t base);
csim_inst_t *csim_new_component_ext(csim_board_t *board, csim_component_t *comp, csim_confs_t confs);
void csim_delete_component(csim_inst_t *inst);
csim_inst_t *csim_find_instance(csim_board_t *board, const char *name);
csim_port_t*csim_find_port(csim_component_t *comp, const char *name);

void csim_default_update(csim_inst_t *inst);
void csim_default_change(csim_iocomp_inst_t *inst, csim_ioinfo_t *state);

void csim_log(csim_board_t *board, csim_level_t level, const char *msg, ...);

void csim_connect(csim_inst_t *inst1, csim_port_t *port1, csim_inst_t *inst2, csim_port_t *port);
void csim_disconnect(csim_inst_t *inst1, csim_port_t *port1, csim_inst_t *inst2, csim_port_t *port);
void csim_mute(csim_inst_t *inst, csim_port_t *port);
void csim_send_digital(csim_inst_t *inst, csim_port_t *port, int digit);

void csim_record_event(csim_board_t *board, csim_evt_t *evt);
void csim_cancel_event(csim_board_t *board, csim_evt_t *evt);

int csim_run(csim_board_t *board, csim_time_t time);
void csim_step(csim_board_t *board);

void csim_no_state(csim_iocomp_inst_t *inst, uint32_t *state);

void csim_wakeup(csim_inst_t *inst);
void csim_port_wakeup(csim_port_inst_t *inst, csim_value_type_t type, csim_value_t val);
void csim_write_wakeup(csim_inst_t *inst, int num, csim_word_t val);

uint8_t csim_byte_at(csim_board_t *board, csim_addr_t addr);
uint16_t csim_half_at(csim_board_t *board, csim_addr_t addr);
uint32_t csim_word_at(csim_board_t *board, csim_addr_t addr);
uint64_t csim_long_at(csim_board_t *board, csim_addr_t addr);

csim_word_t csim_read_io(csim_board_t *board, csim_addr_t addr, int size);
void csim_write_io(csim_board_t *board, csim_addr_t addr, int size, csim_word_t word);
void csim_on_io(csim_addr_t addr, int size, void *data, int access, void *cdata);

void csim_record_iostate(csim_inst_t *inst, csim_iostate_t *state);
void csim_flush_iostates(csim_board_t *board, csim_ioinfo_t infos[]);
void csim_do_input(csim_board_t *board, csim_ioinfo_t *info);

uint32_t csim_parse_uint(const char *str, int *err);

/* core functions */
#define csim_core(i) ((csim_core_t *)(i)->inst.comp)
#define csim_core_pc(i) \
	((csim_core_t *)(i)->inst.comp)->pc(i)
#define csim_core_inst_size(i) \
	((csim_core_t *)(i)->inst.comp)->inst_size(i)
#define csim_core_disasm(i, a, b) \
	((csim_core_t *)(i)->inst.comp)->disasm(i, a, b)
#define csim_core_load(i, p) \
	((csim_core_t *)(i)->inst.comp)->load(i, p)
#define csim_core_memory(i) \
	((csim_core_t *)(i)->inst.comp)->memory(i)

#define csim_port_inst(inst, port) \
	(&inst->ports[port - inst->comp->ports])

#endif	/* CSIM_H */
