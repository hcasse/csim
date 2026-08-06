#
#	CSim component simulator
#	Copyright (C) 2026 University of Toulouse <hugues.casse@irit.fr>
#
#	This program is free software: you can redistribute it and/or modify
#	it under the terms of the GNU General Public License as published by
#	the Free Software Foundation, either version 3 of the License, or
#	(at your option) any later version.
#
#	This program is distributed in the hope that it will be useful,
#	but WITHOUT ANY WARRANTY; without even the implied warranty of
#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#	GNU General Public License for more details.
#
#	You should have received a copy of the GNU General Public License
#	along with this program.  If not, see <https://www.gnu.org/licenses/>.
#

"""Interface with the C python libray."""

import importlib
from csim import driver

class Driver(driver.Driver):
	LIB = None

	def __init__(self):
		if Driver.LIB is None:
			try:
				Driver.LIB = importlib.import_module("libcsim")
			except ImportError as exn:
				raise csim.BoardError(str(exn))

	def register_info(self, reg):
		return self.LIB.register_info(reg)

	def get_register_val(self, inst, reg, index):
		return self.LIB.get_register_val(inst, reg, index)

	def set_register_val(self, inst, reg, index, value):
		self.LIB.set_register_val(inst, reg, index, value)

	def register_make_name(self, inst, reg, index):
		return self.LIB.register_make_name(inst, reg, index)

	def get_comp(self, inst):
		return self.LIB.get_comp(inst)

	def inst_info(self, inst):
		return self.LIB.inst_info(inst)

	def component_info(self, comp):
		return self.LIB.component_info(comp)

	def get_register(self, comp, index):
		return self.LIB.get_register(comp, index)

	def inst_confs(self, inst):
		return self.LIB.inst_confs(inst)

	def get_core(self, board):
		return self.LIB.get_core(board)

	def core_load(self, core, path):
		return self.LIB.core_load(core, path)

	def core_pc(self, core):
		return self.LIB.core_pc(core)

	def core_inst_size(self, core):
		"""Get the size of the current instruction."""
		return None

	def core_disasm(self, core, addr):
		return self.LIB.core_disasm(core, addr)

	def set_break(self, addr):
		self.LIB.set_break(addr)

	def clear_break(self, addr):
		self.LIB.clear_break(addr)

	def do_input(self, board, inst, ress, state):
		"""Perform an input."""
		self.LIB.do_input(board, inst, ress, state)

	def load_board(self, path):
		"""Load the board at the path and return it or None if there is an error."""
		return self.LIB.load_board(path)

	def get_insts(self, board):
		"""Get the instances of the board."""
		return self.LIB.get_insts(board)

	def board_confs(self, board):
		"""Get configuration of the board."""
		return self.LIB.board_confs(board)

	def step(self, board):
		"""Perform one step of simulation."""
		self.LIB.step(board)

	def get_clock(self, board):
		"""Get the clock of the board."""
		return self.LIB.get_clock(board)

	def find_port(self, comp, name):
		"""Find a port by its name."""
		return self.LIB.find_port(comp, name)

	def reset_board(self, board):
		"""Reset the board."""
		self.LIB.reset_board(board)

	def delete_board(self, board):
		"""Delete the board."""
		self.LIB.delete_board(board)

	def get_date(self, board):
		"""Get the current date."""
		return self.LIB.get_date(board)

	def byte_at(self, board, addr):
		"""Get byte at address."""
		return self.LIB.byte_at(board, addr)

	def half_at(self, board, addr):
		"""Get half-word at address."""
		return self.LIB.half_at(board, addr)

	def word_at(self, board, addr):
		"""Get word at address."""
		return self.LIB.word_at(board, addr)

	def set_log_level(self, board, level):
		"""Set log level."""
		self.LIB.set_log_level(board, level)

	def flush_iostates(self, board):
		"""Get the output stet changes."""
		return self.LIB.flush_iostates(board)

	def board_name(self, board):
		"""Get the name of the board."""
		return self.LIB.board_name(board)

	def run(self, board, time):
		return self.LIB.run(board, time)



