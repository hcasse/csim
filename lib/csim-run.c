/*
 * Component simulator test program
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

#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <termio.h>
#include <unistd.h>

#include "csim.h"

#include "button.h"
#include "led.h"

#include "arm_core.h"

#include "button.h"
#include "led.h"

/****** Board ******/

int VERBOSE = 0;

/****** Simulator ******/
void reset_console() {
    struct termios t;
    tcgetattr(0, &t);
    t.c_lflag |= ECHO | ICANON;
    tcsetattr(0, TCSANOW, &t);
    printf("\033[?25h\n");
    fflush(stdout);
}

void on_control_c(int x) {
    reset_console();
    exit(0);
}

void init_console() {
    struct termios t;
    tcgetattr(0, &t);
    t.c_lflag &= ~(ECHO | ICANON);
    tcsetattr(0, TCSANOW, &t);

    atexit(reset_console);
    signal(SIGINT, on_control_c);
    printf("\033[?25l");
}

// csim_inst_t *led, *button;
// arm_sim_t *sim;
csim_core_inst_t *core;

/**
 * Print the state.
 */
void print_state(csim_board_t *board, int clear) {
    static char buf[256] = "";
    static char cbuf[256];
    static int buf_size = 0;

    // move back to the start of the line
    if (clear) {
        memset(cbuf, '\b', buf_size);
        cbuf[buf_size] = '\0';
        fputs(cbuf, stdout);
    }

    // generate the content
    char *p = buf;
    for (csim_iocomp_inst_t *i = board->iocomps; i != NULL; i = i->next) {
        p += ((csim_iocomp_t *)(i->inst.comp))->display(p, i);
        *p++ = ' ';
    }

    // generate the instruction
    csim_addr_t pc = csim_core_pc(core);
    p += sprintf(p, "%08x ", pc);
    csim_core_disasm(core, pc, p);

    // compute the size
    int size = p - buf + strlen(p);
    if (clear) {
        while (size < buf_size) {
            buf[size] = ' ';
            size++;
        }
        buf[size] = '\0';
    }
    buf_size = size;

    fputs(buf, stdout);
}

/**
 * Display the options.
 */
void print_help() {
    fprintf(stderr, "SYNTAX: csim-run BOARD.yaml EXEC.elf\n");
    fprintf(stderr, "\t-h, -help: displays help message.\n");
    fprintf(stderr, "\t-v: verbose mode.\n");
}

/**
 * Application entry.
 */
int main(int argc, const char *argv[]) {
    const char *exec = NULL, *board_path = NULL;

    /* parse arguments */
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] != '-') {
			if(board_path == NULL)
				board_path = argv[i];
			else if(exec == NULL)
				exec = argv[i];
			else {
                print_help();
                fprintf(stderr, "ERROR: several executable provided: %s\n", argv[1]);
                exit(1);
            }
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "-help") == 0) {
            print_help();
            exit(0);
        }
        else if (strcmp(argv[i], "-v") == 0)
            VERBOSE = 1;
        else {
            print_help();
            fprintf(stderr, "ERROR: unknown option: %s\n", argv[i]);
            exit(1);
        }
    }

    /* check arguments */
	if(board_path == NULL) {
		print_help();
		fprintf(stderr, "ERROR: board needed!\n");
		exit(1);
	}
	if(exec == NULL) {
        print_help();
        fprintf(stderr, "ERROR: executable needed!\n");
        exit(1);
    }

    /* build the board */
    csim_board_t *board;
    //char path[256];
    /*if(board_path == NULL) {
        int l = strlen(exec);
        if(strcmp(".elf", exec + l - 4) == 0) {
            strncpy(path, exec, l - 4);
            path[l - 4] = '\0';
        }
        else
            strcpy(path, exec);
        strcat(path, ".yaml");
        if(VERBOSE)
            fprintf(stderr, "looking for board %s.\n", path);
        if(access(path, R_OK) == 0)
            board_path = path;
    }
    if(board_path == NULL) {
        if (VERBOSE)
            fprintf(stderr, "setting default board!\n");
        board = csim_new_board("default");
        core = (csim_core_inst_t *)csim_new_component(board, &arm_component.comp, "core", 0);
        csim_new_component(board, &led_component.comp, "led", 0xA0000000);
		const char *button_conf[] = {
			"name", "button",
			"base", "B0000000",
			"key", "a",
			NULL
		};
        csim_new_component_ext(board, &button_component.comp, button_conf);
        board->level = CSIM_ERROR;
    }
    else {*/
	csim_level_t level = CSIM_INFO;
	if (VERBOSE) {
		fprintf(stderr, "loading board from %s\n", board_path);
		level = CSIM_DEBUG;
	}
	board = csim_load_board_ext(board_path, level);
	if(board == NULL) {
		fprintf(stderr, "ERROR: cannot load the board!\n");
		exit(3);
	}
	if(board->cores == NULL) {
		fprintf(stderr, "ERROR: no core in this board!\n");
		exit(2);
	}
	else
		core = board->cores;
    //}

    // load the executable
    int rc = csim_core_load(core, exec);
    if (rc != 0) {
        fprintf(stderr, "ERROR: cannot load \"%s\": %d.\n", exec, rc);
        exit(1);
    }

    // initialize input
    fd_set set;
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    init_console();

    // perform I/O
    print_state(board, 0);
    while (1) {

        // update display
        print_state(board, 1);

        // get the key
        FD_ZERO(&set);
        FD_SET(0, &set);
        int n = select(1, &set, NULL, NULL, &tv);
        if (n != 0) {
            char key;
            read(0, &key, 1);
            for (csim_iocomp_inst_t *i = board->iocomps; i != NULL; i = i->next)
                ((csim_iocomp_t *)i->inst.comp)->on_key(key, i);
        }
        csim_run(board, 1);
    }
    return 0;
}
