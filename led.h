#ifndef CSIM_LED_H
#define CSIM_LED_H

#include "csim.h"

extern csim_iocomp_t led_component;

int led_state(csim_inst_t *inst);

#define LED_SHINE	1

#endif	// CSIM_LED_H
