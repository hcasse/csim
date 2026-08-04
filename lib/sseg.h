/*
 * CSIM -- Component simulator
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
#ifndef CSIM_7SEG_H
#define CSIM_7SEG_H

#include "csim.h"

extern csim_iocomp_t sseg_component;

#define SSEG_SEGS	1

#define SSEG_0	0b1110111
#define SSEG_1	0b0100100
#define SSEG_2	0b1011101
#define SSEG_3	0b1101101
#define SSEG_4	0b0101110
#define SSEG_5	0b1101011
#define SSEG_6	0b1111011
#define SSEG_7	0b0100101
#define SSEG_8	0b1111111
#define SSEG_9	0b1101111


#endif	// CSIM_7SEG_H

