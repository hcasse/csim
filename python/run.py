#!/usr/bin/env python3

import argparse
import os
import sys

sys.setdlopenflags(os.RTLD_NOW | os.RTLD_GLOBAL)

from csim import Board, BoardError, CSIM_DEBUG
from csim.ui.app import MyApp

# parse arguments
parser = argparse.ArgumentParser(
    description="Simulate an embedded board")
parser.add_argument("board",
    help="YAML file describing the board.")
parser.add_argument("exec",
    help="ELF executable to run the board.")
parser.add_argument("--debug", action="store_true",
    help="Enable debug mode.")
args = parser.parse_args()

board_path = args.board
bin_path = args.exec

# main program
try:
	board = Board(board_path, bin_path)
	if args.debug:
		board.set_log_level(CSIM_DEBUG)
	MyApp(board).run(debug=False)
except BoardError as e:
	print("ERROR:", str(e))
