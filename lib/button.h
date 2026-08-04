#ifndef CSIM_BUTTON_H
#define CSIM_BUTTON_H

#include "csim.h"


extern csim_iocomp_t button_component;

int button_get(csim_inst_t *inst);
void button_set(csim_inst_t *inst, int pushed);


#endif	// CSIM_BUTTON_H

