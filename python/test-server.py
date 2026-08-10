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
driver.set_log_level(board, server.Log.DEBUG)

for inst in driver.get_insts(board):
	comp = driver.get_comp(inst)
	comp_info = driver.component_info(comp)
	inst_info = driver.inst_info(inst)
	print(f"CLIENT: instance {hex(inst)} ({hex(comp), {inst_info}})")
	print(f"\t{comp_info}")

#driver.release()
driver.log("CLIENT: leaving\n")

