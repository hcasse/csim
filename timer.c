/* Generated by gliss-csim (124/05/07 16:37:37) copyright (c) 2024 IRIT - UPS */

#include <stdio.h>
#include <stdlib.h>
#include <csim.h>
#include "csim-rt.h"

#define ____COMP_NUM		(inst->number)
#	define _START_VAL_HIGH	__inst->START_VAL_HIGH
#	define _START_VAL_LOW	__inst->START_VAL_LOW
#	define _STATUS	__inst->STATUS
#	define _CONTROL	__inst->CONTROL
#	define _COUNTER_SNAPSHOT_LOW	__inst->COUNTER_SNAPSHOT_LOW
#	define _COUNTER_SNAPSHOT_HIGH	__inst->COUNTER_SNAPSHOT_HIGH



#define __now inst->board->date


/**
 * Instance definition.
 */
typedef struct  timer_inst_t {
	csim_inst_t inst;
			uint32_t START_VAL_HIGH;
			uint32_t START_VAL_LOW;
			uint32_t STATUS;
			uint32_t CONTROL;
			uint32_t COUNTER_SNAPSHOT_LOW;
			uint32_t COUNTER_SNAPSHOT_HIGH;

;
}  timer_inst_t;


/* pre-definitiion of port update functions */


/* pre-definitiion of event update functions */
	static void on_update_run(csim_inst_t *inst);


/* pre-definitiion of event trigger functions */
	static void on_trigger_run(csim_evt_t *evt);


static void on_update_all(csim_inst_t *inst) {
	
		on_update_run(inst);
	
}


/**
 * Reset the instance.
 * @param inst	Instance to reset.
 */
static void  timer_reset(csim_inst_t *inst) {
    timer_inst_t * timer_inst = ( timer_inst_t *)inst;
}

/**
 * Construct an instance.
 * @param inst	Instance to construct.
 */
static void timer_construct(csim_inst_t *inst, csim_confs_t confs) {
    timer_inst_t * timer_inst = ( timer_inst_t *)inst;
    timer_reset(inst);
}

/**
 * Destruct the instance.
 * @param inst	Instance to destruct.
 */
static void timer_destruct(csim_inst_t *inst) {
}


/* START_VAL_HIGH register functions */

static void name_START_VAL_HIGH(csim_inst_t *inst, int num, char *__buffer, int size) {
	timer_inst_t *__inst = (timer_inst_t *)inst;
	snprintf(__buffer, size, "timer%d_START_VAL_HIGH", inst->number);
}

static void display_START_VAL_HIGH(csim_inst_t *inst, int num, char *__buffer, int size) {
	timer_inst_t *__inst = (timer_inst_t *)inst;
	snprintf(__buffer, size, "%d", __inst->START_VAL_HIGH);;
}

static csim_word_t read_START_VAL_HIGH(csim_inst_t *inst, int num) {
		timer_inst_t *__inst = (timer_inst_t *)inst;
				return __inst->START_VAL_HIGH;
}
 
static csim_word_t get_START_VAL_HIGH(csim_inst_t *inst, int num) {
		timer_inst_t *__inst = (timer_inst_t *)inst;
			return __inst->START_VAL_HIGH;
}
 
static void set_START_VAL_HIGH(csim_inst_t *inst, int num, csim_word_t val) {
	timer_inst_t *__inst = (timer_inst_t *)inst;
		if(__inst->START_VAL_HIGH != val) {
			__inst->START_VAL_HIGH = val;
			on_update_all(inst);
		}
	
}

static void write_START_VAL_HIGH(csim_inst_t *inst, int num, csim_word_t val) {
	timer_inst_t *__inst = (timer_inst_t *)inst;
	set_START_VAL_HIGH(inst, num, val);

		;
		on_update_all(inst);
	
}

/* START_VAL_LOW register functions */

static void name_START_VAL_LOW(csim_inst_t *inst, int num, char *__buffer, int size) {
	timer_inst_t *__inst = (timer_inst_t *)inst;
	snprintf(__buffer, size, "timer%d_START_VAL_LOW", inst->number);
}

static void display_START_VAL_LOW(csim_inst_t *inst, int num, char *__buffer, int size) {
	timer_inst_t *__inst = (timer_inst_t *)inst;
	snprintf(__buffer, size, "%d", __inst->START_VAL_LOW);;
}

static csim_word_t read_START_VAL_LOW(csim_inst_t *inst, int num) {
		timer_inst_t *__inst = (timer_inst_t *)inst;
				return __inst->START_VAL_LOW;
}
 
static csim_word_t get_START_VAL_LOW(csim_inst_t *inst, int num) {
		timer_inst_t *__inst = (timer_inst_t *)inst;
			return __inst->START_VAL_LOW;
}
 
static void set_START_VAL_LOW(csim_inst_t *inst, int num, csim_word_t val) {
	timer_inst_t *__inst = (timer_inst_t *)inst;
		if(__inst->START_VAL_LOW != val) {
			__inst->START_VAL_LOW = val;
			on_update_all(inst);
		}
	
}

static void write_START_VAL_LOW(csim_inst_t *inst, int num, csim_word_t val) {
	timer_inst_t *__inst = (timer_inst_t *)inst;
	set_START_VAL_LOW(inst, num, val);

		;
		on_update_all(inst);
	
}

/* STATUS register functions */

static void name_STATUS(csim_inst_t *inst, int num, char *__buffer, int size) {
	timer_inst_t *__inst = (timer_inst_t *)inst;
	snprintf(__buffer, size, "timer%d_STATUS", inst->number);
}

static void display_STATUS(csim_inst_t *inst, int num, char *__buffer, int size) {
	timer_inst_t *__inst = (timer_inst_t *)inst;
	snprintf(__buffer, size, "%d", __inst->STATUS);;
}

static csim_word_t read_STATUS(csim_inst_t *inst, int num) {
		timer_inst_t *__inst = (timer_inst_t *)inst;
				return __inst->STATUS;
}
 
static csim_word_t get_STATUS(csim_inst_t *inst, int num) {
		timer_inst_t *__inst = (timer_inst_t *)inst;
			return __inst->STATUS;
}
 
static void set_STATUS(csim_inst_t *inst, int num, csim_word_t val) {
	timer_inst_t *__inst = (timer_inst_t *)inst;
		if(__inst->STATUS != val) {
			__inst->STATUS = val;
			on_update_all(inst);
		}
	
}

static void write_STATUS(csim_inst_t *inst, int num, csim_word_t val) {
	timer_inst_t *__inst = (timer_inst_t *)inst;
	set_STATUS(inst, num, val);

		;
		on_update_all(inst);
	
}

/* CONTROL register functions */

static void name_CONTROL(csim_inst_t *inst, int num, char *__buffer, int size) {
	timer_inst_t *__inst = (timer_inst_t *)inst;
	snprintf(__buffer, size, "timer%d_CONTROL", inst->number);
}

static void display_CONTROL(csim_inst_t *inst, int num, char *__buffer, int size) {
	timer_inst_t *__inst = (timer_inst_t *)inst;
	snprintf(__buffer, size, "%d", __inst->CONTROL);;
}

static csim_word_t read_CONTROL(csim_inst_t *inst, int num) {
		timer_inst_t *__inst = (timer_inst_t *)inst;
				return __inst->CONTROL;
}
 
static csim_word_t get_CONTROL(csim_inst_t *inst, int num) {
		timer_inst_t *__inst = (timer_inst_t *)inst;
			return __inst->CONTROL;
}
 
static void set_CONTROL(csim_inst_t *inst, int num, csim_word_t val) {
	timer_inst_t *__inst = (timer_inst_t *)inst;
		if(__inst->CONTROL != val) {
			__inst->CONTROL = val;
			on_update_all(inst);
		}
	
}

static void write_CONTROL(csim_inst_t *inst, int num, csim_word_t val) {
	timer_inst_t *__inst = (timer_inst_t *)inst;
	set_CONTROL(inst, num, val);

			if((((uint32_t)(_bit32(_CONTROL, 2))) == 1)) {
		csim_evt_t *run = malloc(sizeof(csim_evt_t));
			run->date = __now +((_START_VAL_HIGH << 16) + _START_VAL_LOW);
			run->inst = inst;
			run->trigger = on_trigger_run;
			csim_record_event(inst -> board, run);
		_STATUS = _set_field32(_STATUS, 1, 1, 1);
		_CONTROL = _set_field32(_CONTROL, 0, 2, 2);
	}
	if((((uint32_t)(_bit32(_CONTROL, 3))) == 1)) {
		csim_evt_t *run = malloc(sizeof(csim_evt_t));
			run->inst = inst;
			run->trigger = on_trigger_run;
			csim_cancel_event(inst -> board, run);
		_STATUS = _set_field32(_STATUS, 0, 1, 1);
		_CONTROL = _set_field32(_CONTROL, 0, 3, 3);
	}
;
		on_update_all(inst);
	
}

/* COUNTER_SNAPSHOT_LOW register functions */

static void name_COUNTER_SNAPSHOT_LOW(csim_inst_t *inst, int num, char *__buffer, int size) {
	timer_inst_t *__inst = (timer_inst_t *)inst;
	snprintf(__buffer, size, "timer%d_COUNTER_SNAPSHOT_LOW", inst->number);
}

static void display_COUNTER_SNAPSHOT_LOW(csim_inst_t *inst, int num, char *__buffer, int size) {
	timer_inst_t *__inst = (timer_inst_t *)inst;
	snprintf(__buffer, size, "%d", __inst->COUNTER_SNAPSHOT_LOW);;
}

static csim_word_t read_COUNTER_SNAPSHOT_LOW(csim_inst_t *inst, int num) {
		timer_inst_t *__inst = (timer_inst_t *)inst;
				return __inst->COUNTER_SNAPSHOT_LOW;
}
 
static csim_word_t get_COUNTER_SNAPSHOT_LOW(csim_inst_t *inst, int num) {
		timer_inst_t *__inst = (timer_inst_t *)inst;
			return __inst->COUNTER_SNAPSHOT_LOW;
}
 
static void set_COUNTER_SNAPSHOT_LOW(csim_inst_t *inst, int num, csim_word_t val) {
	timer_inst_t *__inst = (timer_inst_t *)inst;
		if(__inst->COUNTER_SNAPSHOT_LOW != val) {
			__inst->COUNTER_SNAPSHOT_LOW = val;
			on_update_all(inst);
		}
	
}

static void write_COUNTER_SNAPSHOT_LOW(csim_inst_t *inst, int num, csim_word_t val) {
	timer_inst_t *__inst = (timer_inst_t *)inst;
	set_COUNTER_SNAPSHOT_LOW(inst, num, val);

			_COUNTER_SNAPSHOT_LOW = ((uint32_t)(_field32_inverted(__now, 16, 0)));
	_COUNTER_SNAPSHOT_HIGH = ((uint32_t)(_field32_inverted(__now, 16, 0)));
;
		on_update_all(inst);
	
}

/* COUNTER_SNAPSHOT_HIGH register functions */

static void name_COUNTER_SNAPSHOT_HIGH(csim_inst_t *inst, int num, char *__buffer, int size) {
	timer_inst_t *__inst = (timer_inst_t *)inst;
	snprintf(__buffer, size, "timer%d_COUNTER_SNAPSHOT_HIGH", inst->number);
}

static void display_COUNTER_SNAPSHOT_HIGH(csim_inst_t *inst, int num, char *__buffer, int size) {
	timer_inst_t *__inst = (timer_inst_t *)inst;
	snprintf(__buffer, size, "%d", __inst->COUNTER_SNAPSHOT_HIGH);;
}

static csim_word_t read_COUNTER_SNAPSHOT_HIGH(csim_inst_t *inst, int num) {
		timer_inst_t *__inst = (timer_inst_t *)inst;
				return __inst->COUNTER_SNAPSHOT_HIGH;
}
 
static csim_word_t get_COUNTER_SNAPSHOT_HIGH(csim_inst_t *inst, int num) {
		timer_inst_t *__inst = (timer_inst_t *)inst;
			return __inst->COUNTER_SNAPSHOT_HIGH;
}
 
static void set_COUNTER_SNAPSHOT_HIGH(csim_inst_t *inst, int num, csim_word_t val) {
	timer_inst_t *__inst = (timer_inst_t *)inst;
		if(__inst->COUNTER_SNAPSHOT_HIGH != val) {
			__inst->COUNTER_SNAPSHOT_HIGH = val;
			on_update_all(inst);
		}
	
}

static void write_COUNTER_SNAPSHOT_HIGH(csim_inst_t *inst, int num, csim_word_t val) {
	timer_inst_t *__inst = (timer_inst_t *)inst;
	set_COUNTER_SNAPSHOT_HIGH(inst, num, val);

		;
		on_update_all(inst);
	
}




/**
 * Array of registers.
 */
static csim_reg_t regs[] = {
	{
		"START_VAL_HIGH",		// name
		0xc,		// offset
		4,		// size
		1,		// count
		4,		// stride
		0,				// flags
		CSIM_INT,		// signal type
		name_START_VAL_HIGH,
		display_START_VAL_HIGH,
		read_START_VAL_HIGH,
		write_START_VAL_HIGH,
		get_START_VAL_HIGH,
		set_START_VAL_HIGH
	},
	{
		"START_VAL_LOW",		// name
		0x8,		// offset
		4,		// size
		1,		// count
		4,		// stride
		0,				// flags
		CSIM_INT,		// signal type
		name_START_VAL_LOW,
		display_START_VAL_LOW,
		read_START_VAL_LOW,
		write_START_VAL_LOW,
		get_START_VAL_LOW,
		set_START_VAL_LOW
	},
	{
		"STATUS",		// name
		0x0,		// offset
		4,		// size
		1,		// count
		4,		// stride
		0,				// flags
		CSIM_INT,		// signal type
		name_STATUS,
		display_STATUS,
		read_STATUS,
		write_STATUS,
		get_STATUS,
		set_STATUS
	},
	{
		"CONTROL",		// name
		0x4,		// offset
		4,		// size
		1,		// count
		4,		// stride
		0,				// flags
		CSIM_INT,		// signal type
		name_CONTROL,
		display_CONTROL,
		read_CONTROL,
		write_CONTROL,
		get_CONTROL,
		set_CONTROL
	},
	{
		"COUNTER_SNAPSHOT_LOW",		// name
		0x10,		// offset
		4,		// size
		1,		// count
		4,		// stride
		0,				// flags
		CSIM_INT,		// signal type
		name_COUNTER_SNAPSHOT_LOW,
		display_COUNTER_SNAPSHOT_LOW,
		read_COUNTER_SNAPSHOT_LOW,
		write_COUNTER_SNAPSHOT_LOW,
		get_COUNTER_SNAPSHOT_LOW,
		set_COUNTER_SNAPSHOT_LOW
	},
	{
		"COUNTER_SNAPSHOT_HIGH",		// name
		0x14,		// offset
		4,		// size
		1,		// count
		4,		// stride
		0,				// flags
		CSIM_INT,		// signal type
		name_COUNTER_SNAPSHOT_HIGH,
		display_COUNTER_SNAPSHOT_HIGH,
		read_COUNTER_SNAPSHOT_HIGH,
		write_COUNTER_SNAPSHOT_HIGH,
		get_COUNTER_SNAPSHOT_HIGH,
		set_COUNTER_SNAPSHOT_HIGH
	},

};


/* predeclaration of port-functions */



/**
 * Array of ports.
 */
static csim_port_t ports[] = {

};




/*run event functions */

static void on_update_run(csim_inst_t *inst) {
	timer_inst_t *__inst = (timer_inst_t *)inst;

	
}

static void on_trigger_run(csim_evt_t *evt) {
	timer_inst_t *__inst = (timer_inst_t *)evt->inst;
	csim_inst_t *inst = evt->inst;

		_STATUS = _set_field32(_STATUS, 1, 0, 0);
	if((((uint32_t)(_bit32(_CONTROL, 1))) == 1)) {
		csim_evt_t *run = malloc(sizeof(csim_evt_t));
			run->date = __now +((_START_VAL_HIGH << 16) + _START_VAL_LOW);
			run->inst = inst;
			run->trigger = on_trigger_run;
			csim_record_event(inst -> board, run);
	}

}



/**
 * timer description structure.
 */
csim_component_t  timer_component = {
    "timer",
    CSIM_SIMPLE,
    0,					// version
    regs,				// registers
    6 ,	// register count
    ports,				// ports
    0,		// port count
    sizeof(timer_inst_t),
    timer_construct,
    timer_destruct,
    timer_reset
};