# Description of Datastructures

## Component

```c
struct csim_component_t {
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
```

Represents a model of a component. A component can be instantiated as an
instance `csim_inst_t`.

* `name`: name of the component.
* `type`: type of the component (see below).
* `version`: version of the component (from 0).
* `regs`: array of IO registers (whose size is `reg_cnt`).
* `reg_cnt`: count of registers in `regs`.
* `ports`: array of ports (whose size is `port_cnt`).
* `port_cnt`: count of ports in `ports`.
* `size`: size of the instances for this component (embedding custom attributes
	of a component).
* `construct`: function called when a new of a component is built.
* `destrcut`: function called when an instance is deleted.
* `reset`: function called to reset an instance.
* `update`: function called to update the state of an instance.


## Component Instance

```c
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
	const char **params;
};
```

Represents the instance of a component.

* `next`: to link the instance in the link list of instance (deprecated).
* `next_core`: only for core instance, to link the list of cores.
* `next_pending`: to link instance that needs to be updated.
* `comp`: component of the instance.
* `base`: base address for the IO registers of the component.
* `name`: instance name.
* `number`: number of the instance among all instances of the component.
	(allows to make names like `PIOA`, `PIOB`, ...).
* `flags`: an OR combination of flags `CSIM_PENDING`.
* `id`: unique number of the instance among all instances (in range 0 to numer of instances).
* `board`: board containing the instance.
* `ports`: array of port instances.
* `params`: array of string ended by a NULL made of pairs of strings (id, value).
	Parameters passed in the configuration.

Flags encompasses:
* `CSIM_PENDING`: marks an instance that needs to be updated.


## Board

```c
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
```

Represents a board i.e. a collection of component instances with at least one,
links between ports of component instances and memory-mapped registers.

* `name`: name of the board
* `insts`: array of instances (of size `inst_cap` but only used until index `inst_cnt`).
* `inst_cap`: size of array `insts`.
* `inst_cnt`: count of entries used in `insts`.
* `cores`: head of linked-list of core instances.
* `iocomps`: head of linked-list of input/output instances.
* `clock`: main clock of the board (in Hz) used as base-time for simulation.
* `date`: current date (in cycle) of the simulation sice the last reset.
* `evts`: head of sorted linked-list of events that are triggered at some date.
* `level`: logging level.
* `pending`: head of linked-list of instances that needs to be updated.
* `iostates_head`: head of linked-list of input/output state that have changed.
* `iostates_count`: count of IO states in the list.
* `confs`: configurations to load the board.
* `log`: function called to perform a log.
* `ios` (deprecated)

The loggin levels encompasses:
* `CSIM_FATAL`: log only fatal errors.
* `CSIM_ERROR`: log also errors.
* `CSIM_WARN`: log also warnings.
* `CSIM_INFO`: log also information.
* `CSIM_DEBUG`: log also debugging messages.
* `CSIM_NOLOG`: no logging


## Register

Represents a component register possibly mapped in memory:
* `name`: register name
* `offset`: offset of the register relatively to the component base.
* `size`: size in bits of register values.
* `count`: for an array of registers, the count of registers in the array.
* `stride`: for an array of registers, the size between register addresses.
* `flags`: register flags.
* `type`: type of values in the register.
* `make_name`: build the name of the register relatively to the component instance.
* `display`: generate the display of the value of a register.
* `read`: read the value of the register from a memory access.
* `write`: write the value to the register from a memory access.
* `get`: get the value of the register without memory access.
* `set`: set the value of the register without memory access.
