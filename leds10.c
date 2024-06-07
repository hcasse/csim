/* Generated by gliss-csim (124/05/07 15:42:05) copyright (c) 2024 IRIT - UPS */

#include <stdio.h>
#include <stdlib.h>
#include <csim.h>
#include "csim-rt.h"

#define ____COMP_NUM		(inst->number)
#	define _LEDR	__inst->LEDR

#	define LEDP_BASE	0
#	define _LEDP	__inst->LEDP
#	define LEDP_PORT	&ports[LEDP_BASE]
#		define LEDP_SET(x)	\
			{ \
				if(x != _LEDP) { \ 
					_LEDP = x; \
					csim_send_digital(inst, LEDP_PORT, _LEDP); \
				} \
			}



/**
 * Instance definition.
 */
typedef struct  leds10_inst_t {
    csim_iocomp_t inst;
			uint32_t LEDR;

			uint32_t LEDP;
;
}  leds10_inst_t;


/* pre-definitiion of update functions */
	static void on_update_LEDP(csim_inst_t *inst);


static void on_update_all(csim_inst_t *inst) {
		on_update_LEDP(inst);
	
}


/**
 * Reset the instance.
 * @param inst	Instance to reset.
 */
static void  leds10_reset(csim_inst_t *inst) {
    leds10_inst_t * leds10_inst = ( leds10_inst_t *)inst;
	leds10_inst -> LEDR = 3;

}

/**
 * Construct an instance.
 * @param inst	Instance to construct.
 */
static void leds10_construct(csim_inst_t *inst, csim_confs_t confs) {
    leds10_inst_t * leds10_inst = ( leds10_inst_t *)inst;
    leds10_reset(inst);
}

/**
 * Destruct the instance.
 * @param inst	Instance to destruct.
 */
static void leds10_destruct(csim_inst_t *inst) {
}


/* LEDR register functions */

static void name_LEDR(csim_inst_t *inst, int num, char *__buffer, int size) {
	leds10_inst_t *__inst = (leds10_inst_t *)inst;
	snprintf(__buffer, size, "leds10%d_LEDR", inst->number);
}


static void display_LEDR(csim_inst_t *inst, int num, char *__buffer, int size) {
	leds10_inst_t *__inst = (leds10_inst_t *)inst;
	snprintf(__buffer, size, "%d", __inst->LEDR);;
}

static csim_word_t read_LEDR(csim_inst_t *inst, int num) {
		leds10_inst_t *__inst = (leds10_inst_t *)inst;
				return __inst->LEDR;
}
 
static csim_word_t get_LEDR(csim_inst_t *inst, int num) {
		leds10_inst_t *__inst = (leds10_inst_t *)inst;
			return __inst->LEDR;
}
 
static void set_LEDR(csim_inst_t *inst, int num, csim_word_t val) {
	leds10_inst_t *__inst = (leds10_inst_t *)inst;
		if(__inst->LEDR != val) {
			__inst->LEDR = val;
			on_update_all(inst);
		}
	
}

static void write_LEDR(csim_inst_t *inst, int num, csim_word_t val) {
	leds10_inst_t *__inst = (leds10_inst_t *)inst;
		set_LEDR(inst, num, val);
	
}




/**
 * Array of registers.
 */
static csim_reg_t regs[] = {
	{
		"LEDR",		// name
		0x0,		// offset
		4,		// size
		1,		// count
		4,		// stride
		0,				// flags
		CSIM_INT,		// signal type
		name_LEDR,
		display_LEDR,
		read_LEDR,
		write_LEDR,
		get_LEDR,
		set_LEDR
	},

};


/* predeclaration of port-functions */
static void on_input_LEDP(csim_inst_t *inst, csim_value_type_t type, csim_value_t val);



/**
 * Array of ports.
 */
static csim_port_t ports[] = {
			{
				"LEDP",
				CSIM_DIGITAL,
				on_input_LEDP
			},

};


/* LEDP port functions */

static void on_input_LEDP(csim_inst_t *inst, csim_value_type_t type, csim_value_t val) {
	leds10_inst_t *__inst = (leds10_inst_t *)inst;
	uint32_t LEDP = __inst->LEDP;
	int ____INDEX = __inst->inst.comp.ports - (ports + LEDP_BASE);
	if (type == CSIM_DIGITAL)
		LEDP = val.digital;
	if (type == CSIM_ANALOG)
		LEDP = val.analog;
	if (type == CSIM_CLOCK)
		LEDP = val.clock;
	if (type == CSIM_SERIAL)
		LEDP = val.serial;
	
	
	
	_LEDR = LEDP;
	((csim_core_t *) inst -> board -> cores -> inst.comp) -> interrupt(inst -> board -> cores,3);;

}

static void on_update_LEDP(csim_inst_t *inst) {
	leds10_inst_t *__inst = (leds10_inst_t *)inst;

}





static int leds10_display(char *buf, csim_iocomp_inst_t *inst) {
	leds10_inst_t *i = (leds10_inst_t *)inst;
	uint32_t val = i->LEDR;
	char display[] = "[ ] [ ] [ ] [ ] [ ] [ ] [ ] [ ] [ ] [ ]";
	for (int i = 0; i < 10;i++) 
		if ((1<<i) & val) 
			display[4*i + 1] = '*';
	return sprintf(buf,display);
}


static void leds10_on_key(char key, csim_iocomp_inst_t *inst) {
}


/**
 * leds10 description structure.
 */
csim_iocomp_t  leds10_component = {
	{
    "leds10",
    CSIM_IO,
    0,					// version
    regs,				// registers
    1,	// register count
    ports,				// ports
    1,		// port count
    sizeof(leds10_inst_t),
    leds10_construct,
    leds10_destruct,
    leds10_reset
},
leds10_display,
leds10_on_key
};