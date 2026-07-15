#!/usr/bin/env python3

import sys
from csim import Board


board = Board("../samples/sample1.yaml")
print(f"Board: {board}")
core = board.get_core()
print(f"Core: {core}")

# display instances
print("Components:")
for inst in board.get_components():
	print(f"* {inst} ({inst.get_confs()})")

# display registers
registers = core.get_registers()
print(f"Registers: {registers}")
for reg in registers:
	print(f"Register: {reg.get_name()}[{reg.get_count()}]: {reg.get_type().name} @{hex(reg.get_offset())}/{reg.get_stride()}")
	print(f"\t{', '.join(reg.make_name(i) for i in range(reg.get_count()))}")

# load the program
board.load_bin("../samples/sample1.elf")
print("Binary loaded!")

# run a few steps
print("Running:")
for i in range(10):
	pc = core.pc()
	print(f"{hex(pc)}: {core.disasm(pc)}")
	board.step()
