#!/usr/bin/env python3

import sys
from csimui.components import Board


board = Board("../samples/sample1.yaml")
print(f"Board: {board}")
core = board.get_core()
print(f"Core: {core}")

# display registers
registers = core.get_registers()
print(f"Registers: {registers}")
for reg in registers:
	print(f"Register: {reg.get_name()}[{reg.get_count()}]: {reg.get_type().name} @{hex(reg.get_offset())}/{reg.get_stride()}")
	print(f"\t{', '.join(reg.make_name(i) for i in range(reg.get_count()))}")
