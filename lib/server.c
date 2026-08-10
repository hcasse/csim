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

#define OK		0
#define ERROR	1

#define QUIT			0	/* => */
#define LOAD_BOARD		1	/* path: string => code: byte, board: word */
#define GET_INSTS		2	/* board: word => code: byte, cnt: half (inst: word)^cnt */
#define BOARD_NAME		3	/* board: word => code: byte, name: string */
#define GET_CLOCK		4	/* board: word => code: byte, clock: word */
#define SET_LOG_LEVEL	5	/* board: word, code: byte, level: byte => */
#define GET_COMP		6	/* inst: word => code: byte, comp: long */
#define COMPONENT_INFO	7	/* comp: long => code: byte, name: string, type: byte, version: word, register count: half, port count: half, instance size: half. */
#define INST_INFO		8	/* inst: word => code: byte, base: word, name: string, number: half, flags: word, id: half */

#define BOARD_SHIFT		24
#define INST_SHIFT		8
#define PORT_SHIFT		0
#define REG_SHIFT		0
#define MAKE_BOARD(board)				((board) << BOARD_SHIFT)
#define MAKE_INST(board, inst)			(MAKE_BOARD(board)|((inst) << INST_SHIFT))
#define MAKE_PORT(board, inst, port)	(MAKE_INST(board, inst)|((port) << PORT_SHIFT))
#define MAKE_REG(board, inst, reg)		(MAKE_INST(board, inst)|((reg) << PORT_SHIFT))
#define BOARD(id)						(((id) >> BOARD_SHIFT) & 0xff)
#define INST(id)						(((id) >> INST_SHIFT) & 0xffff)
#define PORT(id)						(((id) >> PORT_SHIFT) & 0xff)
#define REG(id)							(((id) >> REG_SHIFT) & 0xff)


csim_board_t *boards[16];
int board_cnt = 0;

uint32_t msg_size = BUFFER_SIZE;
uint32_t msg_avail = 0;
uint8_t *msg_buf, *msg_ptr;

csim_level_t log_level = CSIM_DEBUG;

/**
 * Display log information.
 */
void do_log(csim_level_t level, const char *fmt, ...) {
	static char *label[] = {
		"",
		"DEBUG",
		"INFO",
		"WARNING",
		"ERROR",
		"FATAL"
	};
	if(level >= log_level) {
		fprintf(stderr, "SERVER: %s: ", label[level]);
		va_list args;
		va_start(args, fmt);
		vfprintf(stderr, fmt, args);
		va_end(args);
		fputc('\n', stderr);
		fflush(stderr);
	}
}

/**
 * Reset the message state.
 */
void reset_msg() {
	msg_ptr = msg_buf;
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
	msg_size *= 2;
	uint32_t used = msg_ptr - msg_buf;
	msg_buf = realloc(msg_buf, msg_size);
	msg_ptr = msg_buf + used;
}

/**
 * Put chunk of data in the buffer.
 * @param data	Data to put.
 * @param size	Size of data.
 */
void put_chunk(void *data, int size) {
	while(msg_ptr + size > msg_buf + msg_size)
		extend_msg();
	memcpy(msg_ptr, data, size);
	msg_ptr += size;
}

/**
 * Add a byte to the message.
 * @param byte	Added byte.
 */
void put_byte(uint8_t byte) {
	put_chunk(&byte, sizeof(byte));
}

/**
 * Add an half-word to the message.
 * @param half	Added half-word.
 */
void put_half(uint16_t half) {
	put_chunk(&half, sizeof(half));
}

/**
 * Add a word to the message.
 * @param word	Added word.
 */
void put_word(uint32_t word) {
	put_chunk(&word, sizeof(word));
}

/**
 * Add a long to the message.
 * @param long_	Added long.
 */
void put_long(uint64_t long_) {
	put_chunk(&long_, sizeof(long_));
}

/**
 * Add a string to the message.
 * @param str	Added string.
 */
void put_string(const char *str) {
	uint32_t size = strlen(str) + 1;
	put_word(size);
	put_chunk((void *)str, size);
}

/**
 * Send the built message.
 */
void send_msg() {
	write(OUT, msg_buf, msg_ptr - msg_buf);
}

/**
 * Receive a message.
 */
void receive_msg() {
	msg_avail = read(IN, msg_buf, msg_size);
	if(msg_avail == 0) {
		do_log(CSIM_INFO, "pipe cut. leaving...");
		exit(0);
	}
	msg_ptr = msg_buf;
}

/**
 * Return a block of the provided size from the input.
 * @param size	Size of chunk.
 * @return		Address of the chunk.
 */
void *get_chunk(int size) {

	/* load rest of message */
	if(msg_buf + msg_avail < msg_ptr + size) {

		/* require enlargement */
		if(msg_ptr + size > msg_ptr + msg_size)
			extend_msg();

		/* read missing value */
		while(msg_buf + msg_avail < msg_ptr + size)
			msg_avail += read(IN, msg_buf + msg_avail, msg_size - msg_avail);

	}

	/* return memory */
	void *res = msg_ptr;
	msg_ptr += size;
	return res;
}

/**
 * Get next byte from the message.
 * @return	Read byte.
 */
uint8_t get_byte() {
	return *(uint8_t *)get_chunk(sizeof(uint8_t));
}

/**
 * Get next halfword from the message.
 * @return	Read halfword.
 */
uint16_t get_half() {
	return *(uint16_t *)get_chunk(sizeof(uint16_t));
}

/**
 * Get next word from the message.
 * @return	Read word.
 */
uint32_t get_word() {
	return *(uint32_t *)get_chunk(sizeof(uint32_t));
}

/**
 * Get next long from the message.
 * @return	Read long.
 */
uint64_t get_long() {
	return *(uint64_t *)get_chunk(sizeof(uint64_t));
}

/**
 * Get the string from the message.
 * @return	Found string.
 */
const char *get_string() {
	uint32_t size = get_word();
	return (const char *)get_chunk(size);
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
	do_log(CSIM_INFO, "starting csim-server");
	init_msg();
	while(1) {
		receive_msg();
		uint8_t cmd = get_byte();
		do_log(CSIM_DEBUG, "got command %d", cmd);
		switch(cmd) {

		case QUIT:
			do_log(CSIM_INFO, "quitting!");
			exit(0);
			break;

		case LOAD_BOARD: {
				const char *path = get_string();
				do_log(CSIM_DEBUG, "path=%s", path);
				boards[board_cnt] = csim_load_board(path);
				if(!boards[board_cnt])
					send_error("cannot load");
				else {
					reset_msg();
					put_byte(OK);
					put_word(MAKE_BOARD(board_cnt));
					send_msg();
					board_cnt++;
				}
			}
			break;

		case GET_INSTS: {
				int index = BOARD(get_word());
				assert(index < board_cnt);
				csim_board_t *board = boards[index];
				reset_msg();
				put_byte(OK);
				put_half(board->inst_cnt);
				for(int i = 0; i < board->inst_cnt; i++)
					put_word(MAKE_INST(index, i));
				send_msg();
			}
			break;

		case BOARD_NAME: {
				int index = BOARD(get_word());
				assert(index < board_cnt);
				csim_board_t *board = boards[index];
				reset_msg();
				put_byte(OK);
				put_string(board->name);
				send_msg();
			}
			break;

		case GET_CLOCK: {
				int index = BOARD(get_word());
				assert(index < board_cnt);
				csim_board_t *board = boards[index];
				reset_msg();
				put_byte(OK);
				put_word(board->clock);
				send_msg();
			}
			break;

		case SET_LOG_LEVEL: {
				int index = BOARD(get_word());
				assert(index < board_cnt);
				csim_board_t *board = boards[index];
				board->level = get_byte();
				reset_msg();
				put_byte(OK);
				send_msg();
			}
			break;

		case GET_COMP: {
				uint32_t id = get_word();
				int board_index = BOARD(id);
				assert(board_index < board_cnt);
				csim_board_t *board = boards[board_index];
				int inst_index = INST(id);
				assert(inst_index < board->inst_cnt);
				csim_inst_t *inst = board->insts[inst_index];
				reset_msg();
				put_byte(OK);
				put_long((intptr_t)inst->comp);
				send_msg();
			}
			break;

		case COMPONENT_INFO: {
				csim_component_t *comp = (csim_component_t *)get_long();
				reset_msg();
				put_byte(OK);
				put_string(comp->name);
				put_byte(comp->type);
				put_word(comp->version);
				put_half(comp->reg_cnt);
				put_half(comp->port_cnt);
				put_half(comp->size);
				send_msg();
			}
			break;

		case INST_INFO: {
				uint32_t id = get_word();
				int board_index = BOARD(id);
				assert(board_index < board_cnt);
				csim_board_t *board = boards[board_index];
				int inst_index = INST(id);
				assert(inst_index < board->inst_cnt);
				csim_inst_t *inst = board->insts[inst_index];
				reset_msg();
				put_byte(OK);
				put_word(inst->base);
				put_string(inst->name);
				put_half(inst->number);
				put_half(inst->id);
				send_msg();
			}
			break;

		default:
			do_log(CSIM_ERROR, "unknown command: %d\n", cmd);
			send_error("unknown command");
		}
	}
	return 0;
}
