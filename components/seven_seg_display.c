/* Generated by gliss-csim (124/04/21 15:59:12) copyright (c) 2024 IRIT - UPS */

#include <stdio.h>
#include <stdlib.h>
#include <csim.h>
#include "csim-rt.h"

#define ____COMP_NUM		(inst->number)
#	define _DISPLAY	__inst->DISPLAY

#	define input_BASE	0
#	define _input	__inst->input
#	define input_PORT	&ports[input_BASE]
#		define input_SET(x)	\
			{ \
				if(x != _input) { \ 
					_input = x; \
					csim_send_digital(inst, input_PORT, _input); \
				} \
			}


#define now inst->board->date


/**
 * Instance definition.
 */
typedef struct  seven_seg_display_inst_t {
	csim_inst_t inst;
			uint8_t DISPLAY;

			uint8_t input;
;
}  seven_seg_display_inst_t;


/* pre-definitiion of port update functions */
	static void on_update_input(csim_inst_t *inst);


/* pre-definitiion of event update functions */


/* pre-definitiion of event trigger functions */


static void on_update_all(csim_inst_t *inst) {
		on_update_input(inst);
	
	
}


/**
 * Reset the instance.
 * @param inst	Instance to reset.
 */
static void  seven_seg_display_reset(csim_inst_t *inst) {
    seven_seg_display_inst_t * seven_seg_display_inst = ( seven_seg_display_inst_t *)inst;
}

/**
 * Construct an instance.
 * @param inst	Instance to construct.
 */
static void seven_seg_display_construct(csim_inst_t *inst, csim_confs_t confs) {
    seven_seg_display_inst_t * seven_seg_display_inst = ( seven_seg_display_inst_t *)inst;
    seven_seg_display_reset(inst);
}

/**
 * Destruct the instance.
 * @param inst	Instance to destruct.
 */
static void seven_seg_display_destruct(csim_inst_t *inst) {
}


/* DISPLAY register functions */

static void name_DISPLAY(csim_inst_t *inst, int num, char *__buffer, int size) {
	seven_seg_display_inst_t *__inst = (seven_seg_display_inst_t *)inst;
	snprintf(__buffer, size, "seven_seg_display%d_DISPLAY", inst->number);
}

static void display_DISPLAY(csim_inst_t *inst, int num, char *__buffer, int size) {
	seven_seg_display_inst_t *__inst = (seven_seg_display_inst_t *)inst;
	snprintf(__buffer, size, "%d", __inst->DISPLAY);;
}

static csim_word_t read_DISPLAY(csim_inst_t *inst, int num) {
		seven_seg_display_inst_t *__inst = (seven_seg_display_inst_t *)inst;
				return __inst->DISPLAY;
}
 
static csim_word_t get_DISPLAY(csim_inst_t *inst, int num) {
		seven_seg_display_inst_t *__inst = (seven_seg_display_inst_t *)inst;
			return __inst->DISPLAY;
}
 
static void set_DISPLAY(csim_inst_t *inst, int num, csim_word_t val) {
	seven_seg_display_inst_t *__inst = (seven_seg_display_inst_t *)inst;
		if(__inst->DISPLAY != val) {
			__inst->DISPLAY = val;
			on_update_all(inst);
		}
	
}

static void write_DISPLAY(csim_inst_t *inst, int num, csim_word_t val) {
	seven_seg_display_inst_t *__inst = (seven_seg_display_inst_t *)inst;
		set_DISPLAY(inst, num, val);
	
}




/**
 * Array of registers.
 */
static csim_reg_t regs[] = {
	{
		"DISPLAY",		// name
		0x0,		// offset
		1,		// size
		1,		// count
		1,		// stride
		0,				// flags
		CSIM_INT,		// signal type
		name_DISPLAY,
		display_DISPLAY,
		read_DISPLAY,
		write_DISPLAY,
		get_DISPLAY,
		set_DISPLAY
	},

};


/* predeclaration of port-functions */
static void on_input_input(csim_port_inst_t *inst, csim_value_type_t type, csim_value_t val);



/**
 * Array of ports.
 */
static csim_port_t ports[] = {
			{
				"input",
				CSIM_DIGITAL,
				on_input_input
			},

};


/* input port functions */

static void on_input_input(csim_port_inst_t *inst, csim_value_type_t type, csim_value_t val) {
	seven_seg_display_inst_t *__inst = (seven_seg_display_inst_t *)inst->inst;
	uint8_t input = __inst->input;
	int ____INDEX = inst->port - (ports + input_BASE);
	if (type == CSIM_DIGITAL)
		input = val.digital;
	if (type == CSIM_ANALOG)
		input = val.analog;
	if (type == CSIM_CLOCK)
		input = val.clock;
	if (type == CSIM_SERIAL)
		input = val.serial;
	
	
	
	_DISPLAY = input;

}

static void on_update_input(csim_inst_t *inst) {
	seven_seg_display_inst_t *__inst = (seven_seg_display_inst_t *)inst;

}





/**
 * seven_seg_display description structure.
 */
csim_component_t  seven_seg_display_component = {
    "seven_seg_display",
    CSIM_SIMPLE,
    0,					// version
    regs,				// registers
    1 ,	// register count
    ports,				// ports
    1,		// port count
    sizeof(seven_seg_display_inst_t),
    seven_seg_display_construct,
    seven_seg_display_destruct,
    seven_seg_display_reset
};
