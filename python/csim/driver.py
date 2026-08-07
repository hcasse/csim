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

"""Driver definition."""

class Driver:
	"""Parent class of drivers."""

	def register_info(self, reg):
		"""Get information about register:
		(name, offset, size, count, stride, flags, type)."""
		return None

	def get_register_val(self, inst, reg, index):
		"""Get the value of a register."""
		return None

	def set_register_val(self, inst, reg, index, value):
		"""Set value of a register."""
		pass

	def register_make_name(self, inst, reg, index):
		"""Build the name of a register."""
		return None

	def get_comp(self, inst):
		"""Get the component for an instance."""
		return None

	def inst_info(self, inst):
		"""Get information about an instance."""
		return None

	def component_info(self, comp):
		"""Get information about a component."""
		return None

	def get_register(self, comp, index):
		"""Get register by index."""
		return None

	def inst_confs(self, inst):
		"""Get configuration of an instance: list of pair-number of strings (key, value)."""
		return None

	def get_core(self, board):
		"""Get the core of the boatd."""
		return None

	def core_load(self, core, path):
		"""Load the binary in the memory."""
		pass

	def core_pc(self, core):
		"""Get the value of the PC."""
		return None

	def core_inst_size(self, core):
		"""Get the size of the current instruction."""
		return None

	def core_disasm(self, core, addr):
		"""Disassemble the instruction at the provided address."""
		return None

	def set_break(self, core, addr):
		"""Set the breakpoint at address."""
		pass

	def clear_break(self, core, addr):
		"""Clear the breakpoint at provided address."""
		pass

	def do_input(self, ress, state):
		"""Perform an input."""
		pass

	def load_board(self, path):
		"""Load the board at the path and return it or None if there is an error."""
		return None

	def get_insts(self, board):
		"""Get the instances of the board."""
		return None

	def board_confs(self, board):
		"""Get configuration of the board."""
		pass

	def step(self, board):
		"""Perform one step of simulation."""
		pass

	def get_clock(self, board):
		"""Get the clock of the board."""
		return None

	def find_port(self, comp, name):
		"""Find a port by its name."""
		return None

	def reset_board(self, board):
		"""Reset the board."""
		pass

	def delete_board(self, board):
		"""Delete the board."""
		pass

	def get_date(self, board):
		"""Get the current date."""
		return None

	def byte_at(self, board, addr):
		"""Get byte at address."""
		return None

	def half_at(self, board, addr):
		"""Get half-word at address."""
		return None

	def word_at(self, board, addr):
		"""Get word at address."""
		return None

	def set_log_level(self, board, level):
		"""Set log level."""
		pass

	def flush_iostates(self, board):
		"""Get the output stet changes."""
		return None

	def board_name(self, board):
		"""Get the name of the board."""
		return None

	def run(self, board, time):
		"""Execute code during some time."""
		return None

