/*
 * CSIM Component simulator server
 * Copyright (c) 2019, IRIT - UPS <casse@irit.fr>
 *
 * This file is part of CSIM.
 *
 * CSIM is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * CSIM is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with CSIM; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <assert.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "csim.h"

#define IN	0
#define OUT	1
#define BUFFER_SIZE	2048
#define VERSION	"0.1"
#define OK		'0'
#define ERROR	'!'

static csim_board_t *board = NULL;

uint8_t *msg_buf, *msg_top, *msg_ptr;
uint32_t msg_size = BUFFER_SIZE;

/**
 * Display log information.
 */
void do_log(const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);
}

/**
 * Reset the message state.
 */
void reset_msg() {
	msg_ptr = msg_buf;
	msg_top = msg_ptr + msg_size;
}

/**
 * Initialize the message system.
 */
void init_msg() {
	msg_buf = malloc(msg_size);
	reset_msg();
}

/**
 * Extend the size of the buffer.
 */
void extend_msg() {
	uint32_t new_size = msg_size * 2;
	uint8_t *new_buf = malloc(new_size);
	uint32_t used = msg_ptr - msg_buf;
	memcpy(new_buf, msg_buf, used);
	free(msg_buf);
	msg_buf = new_buf;
	msg_size = new_size;
	msg_ptr = msg_buf + used;
	msg_top = msg_ptr + msg_size;
}

/**
 * Add a byte to the message.
 * @param byte	Added byte.
 */
void put_byte(uint8_t byte) {
	*msg_ptr++ = byte;
}

/**
 * Add an half-word to the message.
 * @param half	Added half-word.
 */
void put_half(uint16_t half) {
	*(uint16_t *)msg_ptr = half;
	msg_ptr += sizeof(uint16_t);
}

/**
 * Add a word to the message.
 * @param word	Added word.
 */
void put_word(uint32_t word) {
	*(uint32_t *)msg_ptr = word;
	msg_ptr += sizeof(uint32_t);
}

/**
 * Add a string to the message.
 * @param str	Added string.
 */
void put_string(const char *str) {
	uint32_t size = strlen(str) + 1;
	memcpy(msg_ptr, str, size);
	msg_ptr += size;
}

/**
 * Send the built message.
 */
void send_msg() {
	*msg_ptr++ = '\n';
	/*char buffer[1024];
	for(int i = 0; i < msg_ptr - msg_buf; i++) {
		uint8_t c = msg_buf[i];
		if(c < 32 || c >= 128)
			sprintf(buffer + 2*i, "%02x", (int)msg_buf[i]);
		else
			sprintf(buffer + 2*i, " %c", c);
	}
	buffer[(msg_ptr - msg_buf) * 2] = '\0';
	do_log("INFO: send message: %s.\n", buffer);*/
	write(OUT, msg_buf, msg_ptr - msg_buf);
}

/**
 * Receive a message.
 */
void receive_msg() {
	int size = read(IN, msg_buf, msg_size);
	if(size == 0) {
		do_log("INFO: Client stopped. leaving...\n");
		exit(0);
	}
	msg_ptr = msg_buf;
}

/**
 * Get next byte from the message.
 * @return	Read byte.
 */
uint8_t get_byte() {
	uint8_t byte = *msg_ptr++;
	return byte;
}

/**
 * Get next halfword from the message.
 * @return	Read halfword.
 */
uint16_t get_half() {
	uint16_t half = *(uint16_t *)msg_ptr;
	msg_ptr += sizeof(uint16_t);
	return half;
}

/**
 * Get next word from the message.
 * @return	Read word.
 */
uint32_t get_word() {
	uint32_t word = *(uint32_t *)msg_ptr;
	msg_ptr += sizeof(uint32_t);
	return word;
}

/**
 * Get the string from the message.
 * @return	Found string.
 */
const char *get_string() {
	const char *str = (const char *)msg_ptr;
	uint32_t size = strlen(str) + 1;
	msg_ptr += size;
	return str;
}

/**
 * Display an error.
 * @param msg	Message to send.
 */
void send_error(const char *msg, ...) {
	char buf[1024];
	va_list args;
	va_start(args, msg);
	reset_msg();
	put_byte(ERROR);
	vsprintf(buf, msg, args);
	put_string(buf);
	send_msg();
}

/**
 * Send an OK message.
 */
void send_ok() {
	reset_msg();
	put_byte(OK);
	send_msg();
}


/**
 * Server entry point.
 */
int main() {
	do_log("starting csim-server\n");
	init_msg();
	while(1) {
		receive_msg();
		uint8_t cmd = get_byte();
		//do_log("INFO: got command '%c'\n", cmd);
		switch(cmd) {

		// load path:STR => null
		case 'L':
			if(board != NULL)
				send_error("Board already loaded!");
			else {
				const char *path = get_string();
				board = csim_load_board(path);
				if(board == NULL)
					send_error("Cannot open \"%s\".", path);
				else {
					assert(board->cores);
					send_ok();
				}
			}
			break;

		// reset board =>
		case '0':
			assert(board != NULL);
			csim_reset_board(board);
			send_ok();
			break;

		// count component => HALF
		case '#':
			assert(board != NULL);
			reset_msg();
			put_byte(OK);
			put_half(board->inst_cnt);
			send_msg();
			break;

		// core index => HALF
		case 'C':
			reset_msg();
			put_byte(OK);
			put_half(board->cores->inst.id);
			send_msg();
			break;

		// get IO components => count: HALF (index: HALF)*
		case 'I': {
				reset_msg();
				put_byte(OK);
				int c = 0;
				for(int i = 0; i < board->inst_cnt; i++)
					if(board->insts[i]->comp->type == CSIM_IO)
						c++;
				put_half(c);
				for(int i = 0; i < board->inst_cnt; i++)
					if(board->insts[i]->comp->type == CSIM_IO)
						put_half(i);
				send_msg();
			}
			break;

		// get component information id: HALF
		// => component name: STR, instance name: STR, type: BYTE, version: WORD,
		// register count: HALF, port count: HALF
		case 'c': {
				assert(board != NULL);
				int i = get_half();
				assert(0 <= i && i < board->inst_cnt);
				csim_inst_t *inst = board->insts[i];
				reset_msg();
				put_byte(OK);
				put_string(inst->comp->name);
				put_string(inst->name);
				put_byte(inst->comp->type);
				put_word(inst->comp->version);
				put_half(inst->comp->reg_cnt);
				put_half(inst->comp->port_cnt);
				send_msg();
			}
			break;

		// get register information index: HALF
		// => name: STR, offset: WORD, size: HALF, count: HALF, stride: WORD,
		// flags: WORD
		case 'R': {
				assert(board != NULL);
				int comp_idx = get_half();
				assert(0 <= comp_idx && comp_idx < board->inst_cnt);
				csim_inst_t *inst = board->insts[comp_idx];
				int reg_idx = get_half();
				assert(0 <= reg_idx && reg_idx < inst->comp->reg_cnt);
				csim_reg_t *reg = &inst->comp->regs[reg_idx];
				reset_msg();
				put_byte(OK);
				put_string(reg->name);
				put_word(reg->offset);
				put_half(reg->size);
				put_half(reg->count);
				put_word(reg->stride);
				put_word(reg->flags);
				put_half(reg->type);
				send_msg();
			}
			break;

		// load binary path: STR =>
		case 'B': {
				assert(board != NULL);
				const char *path = get_string();
				int res = csim_core_load((csim_core_inst_t *)board->cores, path);
				if(res == 0)
					send_ok();
				else
					send_error("Cannot load \"%s\": error %d", path, res);
			}
			break;

		/*case 'i': {
				assert(board != NULL);
				uint32_t addr = get_word();
				unsigned size = csim_core_inst_size();
			}
			break;*/

		// quit
		case 'Q':
			return 0;

		// get version => version: STR
		case 'v':
			reset_msg();
			put_byte(OK);
			put_string("csim-server V" VERSION);
			send_msg();
			break;

		default:
			do_log("unknown command: '%c'\n", cmd);
		}
	}
	return 0;
}
