#!/usr/bin/env python3

import sys
from csim import Board


board = Board("../samples/sample1.yaml")
print(f"Board: {board}")
core = board.get_core()
print(f"Core: {core}")

# load the program
board.load_bin("../samples/break.elf")
print("Binary loaded!")

# set a breakpoint
stop_addr = 0x100b4
core.set_break(stop_addr)
print("Set breakpoint at", hex(stop_addr))

# run a few steps
print("Running:")
while True:
#for i in range(10):
	pc = core.pc()
	print(f"{hex(pc)}: {core.disasm(pc)}")
	res = board.run(1)
	print("DEBUG: python: res =", res)
	if res != 0:
		break


