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

#define CSIM_INSIDE
#include "csim.h"
#include "mem.h"

#include "button.h"
#include "led.h"

#include "arm_core.h"

/* Composants internes ATMEGA328P */
#include "portb.h"
#include "portc.h"
#include "portd.h"
#include "tc16bit1.h"
#include "tc8bit0.h"
#include "oscillator.h"

/*#include "yaml.h"

#include "arm_core.h"*/
#include "button.h"
#include "led.h"

/****** Board ******/

/*csim_component_t *comps[] = {
        &led_component.comp,
        &button_component.comp,
        &arm_component.comp,
        NULL
};*/

/*typedef struct {
        enum {
                TOP,
                IN_COMPS,
                IN_COMP,
                IN_CONNECT,
                IN_LINK
        } state;
        csim_board_t *board;
        csim_memory_t *mem;
        const char *name, *type;
        char key;
        arm_address_t base;
        csim_inst_t *from_inst, *to_inst;
        csim_port_t *from_port, *to_port;
        int conf_cnt;
        char *confs[32];
} loader_t;*/

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
    fprintf(stderr, "SYNTAX: test2 FICHIER.elf\n");
    fprintf(stderr, "\t-b, -board BOARD-PATH: select the board descriptor to use.\n");
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
            if (exec != NULL) {
                print_help();
                fprintf(stderr, "ERROR: several executable provided: %s\n", argv[1]);
                exit(1);
            } else
                exec = argv[i];
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "-help") == 0) {
            print_help();
            exit(0);
        } else if (strcmp(argv[i], "-b") == 0 || strcmp(argv[i], "-board") == 0) {
            i++;
            if (i >= argc) {
                print_help();
                fprintf(stderr, "ERROR: -b, -board requires an argument.\n");
                exit(1);
            }
            board_path = argv[i];
        } else if (strcmp(argv[i], "-v") == 0)
            VERBOSE = 1;
        else {
            print_help();
            fprintf(stderr, "ERROR: unknown option: %s\n", argv[i]);
            exit(1);
        }
    }

    /* check arguments */
    if (exec == NULL) {
        print_help();
        fprintf(stderr, "ERROR: executable needed!\n");
        exit(1);
    }

    /* build the board */
    csim_board_t *board;
    char path[256];
    if (board_path == NULL) {
        int l = strlen(exec);
        if (strcmp(".elf", exec + l - 4) == 0) {
            strncpy(path, exec, l - 4);
            path[l - 4] = '\0';
        } else
            strcpy(path, exec);
        strcat(path, ".yaml");
        if (VERBOSE)
            fprintf(stderr, "looking for board %s.\n", path);
        if (access(path, R_OK) == 0)
            board_path = path;
    }
    if (board_path == NULL) {
        if (VERBOSE)
            fprintf(stderr, "setting default board!\n");
        board = csim_new_board("default", NULL);
        core = (csim_core_inst_t *)csim_new_component(board, &arm_component.comp, "core", 0);
        char *confs[] = {"key", "a", NULL};
        csim_new_component(board, &led_component.comp, "led", 0xA0000000);
        csim_new_component_ext(board, &button_component.comp, "button", 0xB0000000, confs);
        board->level = CSIM_ERROR;
    } else {
        if (VERBOSE)
            fprintf(stderr, "loading board from %s\n", board_path);
        board = csim_load_board(board_path, NULL);
        if (board == NULL) {
            fprintf(stderr, "ERROR: cannot load the board!\n");
            exit(3);
        }
        if (board->cores == NULL) {
            fprintf(stderr, "ERROR: no core in this board!\n");
            exit(2);
        } else
            core = board->cores;
    }

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
