/* Generated by gliss-csim (124/05/01 22:20:40) copyright (c) 2024 IRIT - UPS */

#include <stdio.h>
#include <stdlib.h>
#include <csim.h>
#include "csim-rt.h"

#define ____COMP_NUM		(inst->number)
#	define _HEX03	__inst->HEX03
#	define _HEX45	__inst->HEX45

#	define HEX5_BASE	0
#	define _HEX5	__inst->HEX5
#	define HEX5_PORT	&ports[HEX5_BASE]
#		define HEX5_SET(x)	\
			{ \
				if(x != _HEX5) { \ 
					_HEX5 = x; \
					csim_send_digital(inst, HEX5_PORT, _HEX5); \
				} \
			}
#	define HEX2_BASE	1
#	define _HEX2	__inst->HEX2
#	define HEX2_PORT	&ports[HEX2_BASE]
#		define HEX2_SET(x)	\
			{ \
				if(x != _HEX2) { \ 
					_HEX2 = x; \
					csim_send_digital(inst, HEX2_PORT, _HEX2); \
				} \
			}
#	define HEX1_BASE	2
#	define _HEX1	__inst->HEX1
#	define HEX1_PORT	&ports[HEX1_BASE]
#		define HEX1_SET(x)	\
			{ \
				if(x != _HEX1) { \ 
					_HEX1 = x; \
					csim_send_digital(inst, HEX1_PORT, _HEX1); \
				} \
			}
#	define HEX4_BASE	3
#	define _HEX4	__inst->HEX4
#	define HEX4_PORT	&ports[HEX4_BASE]
#		define HEX4_SET(x)	\
			{ \
				if(x != _HEX4) { \ 
					_HEX4 = x; \
					csim_send_digital(inst, HEX4_PORT, _HEX4); \
				} \
			}
#	define HEX3_BASE	4
#	define _HEX3	__inst->HEX3
#	define HEX3_PORT	&ports[HEX3_BASE]
#		define HEX3_SET(x)	\
			{ \
				if(x != _HEX3) { \ 
					_HEX3 = x; \
					csim_send_digital(inst, HEX3_PORT, _HEX3); \
				} \
			}
#	define HEX0_BASE	5
#	define _HEX0	__inst->HEX0
#	define HEX0_PORT	&ports[HEX0_BASE]
#		define HEX0_SET(x)	\
			{ \
				if(x != _HEX0) { \ 
					_HEX0 = x; \
					csim_send_digital(inst, HEX0_PORT, _HEX0); \
				} \
			}



/**
 * Instance definition.
 */
typedef struct  seven_seg_controller_inst_t {
	csim_inst_t inst;
			uint32_t HEX03;
			uint32_t HEX45;

			uint8_t HEX5;
			uint8_t HEX2;
			uint8_t HEX1;
			uint8_t HEX4;
			uint8_t HEX3;
			uint8_t HEX0;
;
}  seven_seg_controller_inst_t;


/* pre-definitiion of update functions */
	static void on_update_HEX5(csim_inst_t *inst);
	static void on_update_HEX2(csim_inst_t *inst);
	static void on_update_HEX1(csim_inst_t *inst);
	static void on_update_HEX4(csim_inst_t *inst);
	static void on_update_HEX3(csim_inst_t *inst);
	static void on_update_HEX0(csim_inst_t *inst);


static void on_update_all(csim_inst_t *inst) {
		on_update_HEX5(inst);
		on_update_HEX2(inst);
		on_update_HEX1(inst);
		on_update_HEX4(inst);
		on_update_HEX3(inst);
		on_update_HEX0(inst);
	
}


/**
 * Reset the instance.
 * @param inst	Instance to reset.
 */
static void  seven_seg_controller_reset(csim_inst_t *inst) {
    seven_seg_controller_inst_t * seven_seg_controller_inst = ( seven_seg_controller_inst_t *)inst;
	seven_seg_controller_inst -> HEX03 = 0;
	seven_seg_controller_inst -> HEX45 = 0;

}

/**
 * Construct an instance.
 * @param inst	Instance to construct.
 */
static void seven_seg_controller_construct(csim_inst_t *inst, csim_confs_t confs) {
    seven_seg_controller_inst_t * seven_seg_controller_inst = ( seven_seg_controller_inst_t *)inst;
    seven_seg_controller_reset(inst);
}

/**
 * Destruct the instance.
 * @param inst	Instance to destruct.
 */
static void seven_seg_controller_destruct(csim_inst_t *inst) {
}


/* HEX03 register functions */

static void name_HEX03(csim_inst_t *inst, int num, char *__buffer, int size) {
	seven_seg_controller_inst_t *__inst = (seven_seg_controller_inst_t *)inst;
	snprintf(__buffer, size, "seven_seg_controller%d_HEX03", inst->number);
}


static void display_HEX03(csim_inst_t *inst, int num, char *__buffer, int size) {
	seven_seg_controller_inst_t *__inst = (seven_seg_controller_inst_t *)inst;
	snprintf(__buffer, size, "%d", __inst->HEX03);;
}

static csim_word_t read_HEX03(csim_inst_t *inst, int num) {
		seven_seg_controller_inst_t *__inst = (seven_seg_controller_inst_t *)inst;
				return __inst->HEX03;
}
 
static csim_word_t get_HEX03(csim_inst_t *inst, int num) {
		seven_seg_controller_inst_t *__inst = (seven_seg_controller_inst_t *)inst;
			return __inst->HEX03;
}
 
static void set_HEX03(csim_inst_t *inst, int num, csim_word_t val) {
	seven_seg_controller_inst_t *__inst = (seven_seg_controller_inst_t *)inst;
		if(__inst->HEX03 != val) {
			__inst->HEX03 = val;
			on_update_all(inst);
		}
	
}

static void write_HEX03(csim_inst_t *inst, int num, csim_word_t val) {
	seven_seg_controller_inst_t *__inst = (seven_seg_controller_inst_t *)inst;
		set_HEX03(inst, num, val);
	
}

/* HEX45 register functions */

static void name_HEX45(csim_inst_t *inst, int num, char *__buffer, int size) {
	seven_seg_controller_inst_t *__inst = (seven_seg_controller_inst_t *)inst;
	snprintf(__buffer, size, "seven_seg_controller%d_HEX45", inst->number);
}


static void display_HEX45(csim_inst_t *inst, int num, char *__buffer, int size) {
	seven_seg_controller_inst_t *__inst = (seven_seg_controller_inst_t *)inst;
	snprintf(__buffer, size, "%d", __inst->HEX45);;
}

static csim_word_t read_HEX45(csim_inst_t *inst, int num) {
		seven_seg_controller_inst_t *__inst = (seven_seg_controller_inst_t *)inst;
				return __inst->HEX45;
}
 
static csim_word_t get_HEX45(csim_inst_t *inst, int num) {
		seven_seg_controller_inst_t *__inst = (seven_seg_controller_inst_t *)inst;
			return __inst->HEX45;
}
 
static void set_HEX45(csim_inst_t *inst, int num, csim_word_t val) {
	seven_seg_controller_inst_t *__inst = (seven_seg_controller_inst_t *)inst;
		if(__inst->HEX45 != val) {
			__inst->HEX45 = val;
			on_update_all(inst);
		}
	
}

static void write_HEX45(csim_inst_t *inst, int num, csim_word_t val) {
	seven_seg_controller_inst_t *__inst = (seven_seg_controller_inst_t *)inst;
		set_HEX45(inst, num, val);
	
}




/**
 * Array of registers.
 */
static csim_reg_t regs[] = {
	{
		"HEX03",		// name
		0x0,		// offset
		4,		// size
		1,		// count
		4,		// stride
		0,				// flags
		CSIM_INT,		// signal type
		name_HEX03,
		display_HEX03,
		read_HEX03,
		write_HEX03,
		get_HEX03,
		set_HEX03
	},
	{
		"HEX45",		// name
		0x10,		// offset
		4,		// size
		1,		// count
		4,		// stride
		0,				// flags
		CSIM_INT,		// signal type
		name_HEX45,
		display_HEX45,
		read_HEX45,
		write_HEX45,
		get_HEX45,
		set_HEX45
	},

};


/* predeclaration of port-functions */
static void on_input_HEX5(csim_port_inst_t *inst, csim_value_type_t type, csim_value_t val);
static void on_input_HEX2(csim_port_inst_t *inst, csim_value_type_t type, csim_value_t val);
static void on_input_HEX1(csim_port_inst_t *inst, csim_value_type_t type, csim_value_t val);
static void on_input_HEX4(csim_port_inst_t *inst, csim_value_type_t type, csim_value_t val);
static void on_input_HEX3(csim_port_inst_t *inst, csim_value_type_t type, csim_value_t val);
static void on_input_HEX0(csim_port_inst_t *inst, csim_value_type_t type, csim_value_t val);



/**
 * Array of ports.
 */
static csim_port_t ports[] = {
			{
				"HEX5",
				CSIM_DIGITAL,
				on_input_HEX5
			},
			{
				"HEX2",
				CSIM_DIGITAL,
				on_input_HEX2
			},
			{
				"HEX1",
				CSIM_DIGITAL,
				on_input_HEX1
			},
			{
				"HEX4",
				CSIM_DIGITAL,
				on_input_HEX4
			},
			{
				"HEX3",
				CSIM_DIGITAL,
				on_input_HEX3
			},
			{
				"HEX0",
				CSIM_DIGITAL,
				on_input_HEX0
			},

};


/* HEX5 port functions */

static void on_input_HEX5(csim_port_inst_t *inst, csim_value_type_t type, csim_value_t val) {
	seven_seg_controller_inst_t *__inst = (seven_seg_controller_inst_t *)inst->inst;
	uint8_t HEX5 = __inst->HEX5;
	int ____INDEX = inst->port - (ports + HEX5_BASE);
	if (type == CSIM_DIGITAL)
		HEX5 = val.digital;
	if (type == CSIM_ANALOG)
		HEX5 = val.analog;
	if (type == CSIM_CLOCK)
		HEX5 = val.clock;
	if (type == CSIM_SERIAL)
		HEX5 = val.serial;
	
	
	

}

static void on_update_HEX5(csim_inst_t *inst) {
	seven_seg_controller_inst_t *__inst = (seven_seg_controller_inst_t *)inst;
	HEX5_SET(_field32_inverted(_HEX45, 15, 8));

}

/* HEX2 port functions */

static void on_input_HEX2(csim_port_inst_t *inst, csim_value_type_t type, csim_value_t val) {
	seven_seg_controller_inst_t *__inst = (seven_seg_controller_inst_t *)inst->inst;
	uint8_t HEX2 = __inst->HEX2;
	int ____INDEX = inst->port - (ports + HEX2_BASE);
	if (type == CSIM_DIGITAL)
		HEX2 = val.digital;
	if (type == CSIM_ANALOG)
		HEX2 = val.analog;
	if (type == CSIM_CLOCK)
		HEX2 = val.clock;
	if (type == CSIM_SERIAL)
		HEX2 = val.serial;
	
	
	

}

static void on_update_HEX2(csim_inst_t *inst) {
	seven_seg_controller_inst_t *__inst = (seven_seg_controller_inst_t *)inst;
	HEX2_SET(_field32_inverted(_HEX03, 23, 16));

}

/* HEX1 port functions */

static void on_input_HEX1(csim_port_inst_t *inst, csim_value_type_t type, csim_value_t val) {
	seven_seg_controller_inst_t *__inst = (seven_seg_controller_inst_t *)inst->inst;
	uint8_t HEX1 = __inst->HEX1;
	int ____INDEX = inst->port - (ports + HEX1_BASE);
	if (type == CSIM_DIGITAL)
		HEX1 = val.digital;
	if (type == CSIM_ANALOG)
		HEX1 = val.analog;
	if (type == CSIM_CLOCK)
		HEX1 = val.clock;
	if (type == CSIM_SERIAL)
		HEX1 = val.serial;
	
	
	

}

static void on_update_HEX1(csim_inst_t *inst) {
	seven_seg_controller_inst_t *__inst = (seven_seg_controller_inst_t *)inst;
	HEX1_SET(_field32_inverted(_HEX03, 15, 8));

}

/* HEX4 port functions */

static void on_input_HEX4(csim_port_inst_t *inst, csim_value_type_t type, csim_value_t val) {
	seven_seg_controller_inst_t *__inst = (seven_seg_controller_inst_t *)inst->inst;
	uint8_t HEX4 = __inst->HEX4;
	int ____INDEX = inst->port - (ports + HEX4_BASE);
	if (type == CSIM_DIGITAL)
		HEX4 = val.digital;
	if (type == CSIM_ANALOG)
		HEX4 = val.analog;
	if (type == CSIM_CLOCK)
		HEX4 = val.clock;
	if (type == CSIM_SERIAL)
		HEX4 = val.serial;
	
	
	

}

static void on_update_HEX4(csim_inst_t *inst) {
	seven_seg_controller_inst_t *__inst = (seven_seg_controller_inst_t *)inst;
	HEX4_SET(_field32_inverted(_HEX45, 7, 0));

}

/* HEX3 port functions */

static void on_input_HEX3(csim_port_inst_t *inst, csim_value_type_t type, csim_value_t val) {
	seven_seg_controller_inst_t *__inst = (seven_seg_controller_inst_t *)inst->inst;
	uint8_t HEX3 = __inst->HEX3;
	int ____INDEX = inst->port - (ports + HEX3_BASE);
	if (type == CSIM_DIGITAL)
		HEX3 = val.digital;
	if (type == CSIM_ANALOG)
		HEX3 = val.analog;
	if (type == CSIM_CLOCK)
		HEX3 = val.clock;
	if (type == CSIM_SERIAL)
		HEX3 = val.serial;
	
	
	

}

static void on_update_HEX3(csim_inst_t *inst) {
	seven_seg_controller_inst_t *__inst = (seven_seg_controller_inst_t *)inst;
	HEX3_SET(_field32_inverted(_HEX03, 31, 24));

}

/* HEX0 port functions */

static void on_input_HEX0(csim_port_inst_t *inst, csim_value_type_t type, csim_value_t val) {
	seven_seg_controller_inst_t *__inst = (seven_seg_controller_inst_t *)inst->inst;
	uint8_t HEX0 = __inst->HEX0;
	int ____INDEX = inst->port - (ports + HEX0_BASE);
	if (type == CSIM_DIGITAL)
		HEX0 = val.digital;
	if (type == CSIM_ANALOG)
		HEX0 = val.analog;
	if (type == CSIM_CLOCK)
		HEX0 = val.clock;
	if (type == CSIM_SERIAL)
		HEX0 = val.serial;
	
	
	

}

static void on_update_HEX0(csim_inst_t *inst) {
	seven_seg_controller_inst_t *__inst = (seven_seg_controller_inst_t *)inst;
	HEX0_SET(_field32_inverted(_HEX03, 7, 0));

}





/**
 * seven_seg_controller description structure.
 */
csim_component_t  seven_seg_controller_component = {
    "seven_seg_controller",
    CSIM_SIMPLE,
    0,					// version
    regs,				// registers
    2 ,	// register count
    ports,				// ports
    6,		// port count
    sizeof(seven_seg_controller_inst_t),
    seven_seg_controller_construct,
    seven_seg_controller_destruct,
    seven_seg_controller_reset
};
