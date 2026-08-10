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

"""Driver implementation as Python-client with piped C server of CSim.."""

from enum import IntEnum
import os
import sys
import struct
import subprocess

from csim import driver

SERVER_PATH = "../lib/csim-server"

class Log(IntEnum):
	NOLOG = 0
	DEBUG = 1
	INFO = 2
	WARN = 3
	ERROR = 4
	FATAL = 5

class Answer(IntEnum):
	OK = 0
	ERROR = 1

class Command(IntEnum):
	QUIT = 0
	LOAD_BOARD = 1
	GET_INSTS = 2
	BOARD_NAME = 3
	GET_CLOCK = 4
	SET_LOG_LEVEL = 5
	GET_COMP = 6
	COMPONENT_INFO = 7
	INST_INFO = 8


class Driver(driver.Driver):
	"""Parent class of drivers."""

	def __init__(self, server_path = None):
		if server_path is None:
			server_path = SERVER_PATH
		self.server = subprocess.Popen(
			SERVER_PATH,
			stdin=subprocess.PIPE,
			stdout=subprocess.PIPE
		)
		self.input = self.server.stdout
		self.output = self.server.stdin
		self.error_msg = None

	def log(self, msg):
		sys.stderr.write(msg)
		sys.stderr.flush()

	def put_byte(self, value):
		self.output.write(struct.pack("B", value))

	def put_half(self, value):
		self.output.write(struct.pack("H", value))

	def put_word(self, value):
		self.output.write(struct.pack("I", value))

	def put_long(self, value):
		self.output.write(struct.pack("Q", value))

	def put_string(self, s):
		data = s.encode("utf-8")
		self.put_word(len(data) + 1)
		self.output.write(data)
		self.put_byte(0)

	def get_chunk(self, size):
		result = bytearray()
		while len(result) < size:
			data = self.input.read(size - len(result))
			if not data:
				raise EOFError()
			result.extend(data)
		return bytes(result)

	def send(self):
		self.output.flush()

	def get_byte(self):
		return struct.unpack("B", self.get_chunk(1))[0]

	def get_half(self):
		return struct.unpack("H", self.get_chunk(2))[0]

	def get_word(self):
		return struct.unpack("I", self.get_chunk(4))[0]

	def get_long(self):
		return struct.unpack("Q", self.get_chunk(8))[0]

	def get_string(self):
		length = self.get_word()
		return self.get_chunk(length).decode("utf-8")[:-1]

	def release(self):
		self.put_byte(Command.QUIT.value)
		self.send()

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
		self.put_byte(Command.GET_COMP.value)
		self.put_word(inst)
		self.send()
		self.get_byte()
		return self.get_long()

	def inst_info(self, inst):
		"""Get information about an instance."""
		self.put_byte(Command.INST_INFO.value)
		self.put_word(inst)
		self.send()
		self.get_byte()
		base = self.get_word()
		name = self.get_string()
		number = self.get_half()
		id = self.get_half()
		return (base, name, number, id)

	def component_info(self, comp):
		"""Get information about a component."""
		self.put_byte(Command.COMPONENT_INFO.value)
		self.put_long(comp)
		self.send()
		self.get_byte()
		name = self.get_string()
		type = self.get_byte()
		version = self.get_word()
		reg_cnt = self.get_half()
		port_cnt = self.get_half()
		size = self.get_half()
		return (name, type, version, reg_cnt, port_cnt, size)

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
		self.put_byte(Command.LOAD_BOARD.value)
		self.put_string(path)
		self.send()
		code = self.get_byte()
		if code == Answer.OK.value:
			return self.get_word()
		else:
			self.error_msg = self.get_string()
			print(f"Error: {self.error_msg}")
			return None

	def get_insts(self, board):
		self.put_byte(Command.GET_INSTS.value)
		self.put_word(board)
		self.send()
		self.get_byte()
		cnt = self.get_half()
		insts = []
		for i in range(cnt):
			insts.append(self.get_word())
		return insts

	def board_confs(self, board):
		"""Get configuration of the board."""
		pass

	def step(self, board):
		"""Perform one step of simulation."""
		pass

	def get_clock(self, board):
		"""Get the clock of the board."""
		self.put_byte(Command.GET_CLOCK.value)
		self.put_word(board)
		self.send()
		self.get_byte()
		return self.get_word()

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
		self.put_byte(Command.SET_LOG_LEVEL.value)
		self.put_word(board)
		self.put_byte(level.value)
		self.send()
		self.get_byte()

	def flush_iostates(self, board):
		"""Get the output stet changes."""
		return None

	def board_name(self, board):
		"""Get the name of the board."""
		self.put_byte(Command.BOARD_NAME.value)
		self.put_word(board)
		self.send()
		self.get_byte()
		return self.get_string()

	def run(self, board, time):
		"""Execute code during some time."""
		return None


