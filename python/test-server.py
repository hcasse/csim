#!/usr/bin/env python3

import os
import sys
from csim import server

sys.stderr.write("CLIENT: starting\n")
sys.stderr.flush()
driver = server.Driver()
board = driver.load_board("../samples/sample1.yaml")
driver.log(f"CLIENT: board={board}\n")
driver.log(f"CLIENT: board name={driver.board_name(board)}\n")
driver.log(f"CLIENT: board clock={driver.get_clock(board)}\n")
driver.log(f"CLIENT: board date={driver.get_date(board)}\n")
driver.log(f"CLIENT: board confs={driver.board_confs(board)}\n")
driver.set_log_level(board, server.Log.DEBUG.value)

# Core testing
core = driver.get_core(board)
print(f"CLIENT: core = {hex(core)}")

# lists instances
for inst in driver.get_insts(board):
	#print(f"- {hex(inst)}")
	comp = driver.get_comp(inst)
	comp_info = driver.component_info(comp)
	inst_info = driver.inst_info(inst)
	print(f"\nCLIENT: instance {hex(inst)} ({hex(comp)}, {inst_info})")
	print(f"CLIENT:\tcomponent = {comp_info}")
	print(f"CLIENT:\t{driver.inst_confs(inst)}")

	if comp_info[3]:
		print("CLIENT: registers")
		for i in range(comp_info[3]):
			reg = driver.get_register(comp, i)
			reg_info = driver.register_info(reg)
			name = driver.register_make_name(inst, reg, 0)
			print(f"CLIENT: - {hex(reg)} ({reg_info}, 0 -> {name})")
		x = driver.get_register_val(inst, reg, 0)
		driver.set_register_val(inst, reg, 0, x + 1)

# other boards test
driver.reset_board(board)
print(f"CLIENT: byte @0 = {hex(driver.byte_at(board, 0))}")
print(f"CLIENT: half @0 = {hex(driver.half_at(board, 0))}")
print(f"CLIENT: word @0 = {hex(driver.word_at(board, 0))}")

# running the code
print(f"CLIENT: load = {driver.core_load(core, '../samples/sample1.elf')}")
pc = driver.core_pc(core)
print(f"CLIENT: pc = {hex(pc)}")
print(f"CLIENT: inst size = {driver.core_inst_size(core)}")
print(f"CLIENT: disasm = {driver.core_disasm(core, pc)}")

for i in range(10):
	pc = driver.core_pc(core)
	print(f"CLIENT: {hex(pc)}:{driver.core_inst_size(core)}: {driver.core_disasm(core, pc)}")
	driver.step(board)

driver.set_break(board, 0x100)
driver.clear_break(board, 0x100)
driver.do_input(board, 2, 1, 1)
driver.run(board, 10)
print(f"CLIENT: ios = {driver.flush_iostates(board)}")


# clean up
driver.delete_board(board)
driver.release()
driver.log("CLIENT: leaving\n")

