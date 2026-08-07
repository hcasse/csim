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


"""Basic CSIM objects."""

from enum import IntEnum

from csim import lib

def error(msg):
	print("ERROR:", msg)

def warn(msg):
	print("WARNING:", msg)

def fatal(msg):
	error(msg)
	sys.exit(1)

class BoardError(Exception):
	"""Errors from the board."""

	def __init__(self, msg):
		self.msg = msg

	def __str__(self):
		return self.msg


class Confs:
	"""Access to a configuration."""

	def __init__(self, confs):
		self.map = {}
		for i in range(0, len(confs), 2):
			self.map[confs[i]] = confs[i+1]

	def get(self, name, default = None):
		try:
			return self.map[name]
		except KeyError:
			return default

	def get_int(self, name, default = 0):
		try:
			val = self.get(name, default)
			if val is None:
				return None
			else:
				return int(val)
		except ValueError:
			raise BoardError(f"{name} should be a int!")

	def obtain(map, name, msg):
		try:
			return map[name]
		except KeyError:
			raise BoardError(msg)

	def __str__(self):
		return str(self.map)


# Log levels
CSIM_NOLOG = 0
CSIM_DEBUG = 1
CSIM_INFO = 2
CSIM_WARN = 3
CSIM_ERROR = 4
CSIM_FATAL = 5

# Component type
CSIM_SIMPLE = 1
CSIM_CORE = 2
CSIM_IO = 3

# Register type
class RType(IntEnum):
	NONE = 0
	BITS = 1
	INT = 2
	ADDR = 3
	FLOAT32 = 4
	FLOAT64 = 5

class Register:
	"""Representation of a register."""

	def __init__(self, comp, reg, driver):
		self.comp = comp
		self.reg = reg
		self.driver = driver
		self.name = None
		self.offset = None
		self.size = None
		self.count = None
		self.stride = None
		self.flags = None
		self.type = None

	def fill(self):
		(name, offset, size, count, stride, flags, type) = self.driver.register_info(self.reg)
		self.name = name
		self.offset = offset
		self.size = size
		self.count = count
		self.stride = stride
		self.flags = flags
		self.type = RType(type)

	def get_name(self):
		if self.name is None:
			self.fill()
		return self.name

	def get_offset(self):
		if self.offset is None:
			self.fill()
		return self.offset

	def get_count(self):
		if self.count is None:
			self.fill()
		return self.count

	def get_stride(self):
		if self.stride is None:
			self.fill()
		return self.stride

	def get_flags(self):
		if self.flags is None:
			self.fill()
		return self.flags

	def get_type(self):
		if self.type is None:
			self.fill()
		return self.type

	def get_value(self, i):
		"""Get the value of the register."""
		return self.driver.get_register_val(self.comp.inst, self.reg, i)

	def set_value(self, i, x):
		"""Set the value of the register."""
		return self.driver.set_register_val(self.comp.inst, self.reg, i, x)

	def make_name(self, index):
		"""Build the name of an instance of the register."""
		return self.driver.register_make_name(self.comp.inst, self.reg, index)


class Component:
	"""Represents a simple component instance."""

	def __init__(self, board, inst):
		self.board = board
		self.inst = inst
		self.comp = board.driver.get_comp(inst)
		self.info = board.driver.inst_info(inst)
		self.name = self.info[1]
		self.registers = None
		self.id = self.info[4]
		self.comp_name = None
		self.confs = None

	def get_name(self):
		"""Get the name of the component."""
		return self.name

	def get_component_name(self):
		"""Get the name of the component describing this component instance."""
		if self.comp_name is None:
			(name, type, vers, rcnt, pcnt, size) = self.board.driver.component_info(self.comp)
			self.comp_name = name
		return self.comp_name

	def get_id(self):
		return self.id

	def get_registers(self):
		"""Get registers of the component. List of Register objects."""
		if not self.registers:
			(name, type, version, reg_cnt, port_cnt, size) = self.board.driver.component_info(self.comp)
			self.registers = []
			for i in range(reg_cnt):
				reg = self.board.driver.get_register(self.comp, i)
				self.registers.append(Register(self, reg, self.board.driver))
		return self.registers

	def __str__(self):
		return f"{self.name}[{self.id}]: {self.get_component_name()}"

	def get_confs(self):
		"""Get the configurations of the component."""
		if self.confs is None:
			self.confs = Confs(self.board.driver.inst_confs(self.inst))
		return self.confs


class Core(Component):
	"""Represents a core component."""

	def __init__(self, board, inst):
		Component.__init__(self, board, inst)
		self.core = self.board.driver.get_core(board.board)
		assert self.core

	def load(self, path):
		"""Load the binary from the path.
		Raises BoardError if there is an error."""
		res = self.board.driver.core_load(self.core, path)
		if res != 0:
			raise BoardError(f'cannot load "{path}"')

	def pc(self):
		"""Get the current PC."""
		return self.board.driver.core_pc(self.core)

	def inst_size(self):
		"""Get the size of the current instruction."""
		return self.board.driver.core_inst_size(self.core)

	def disasm(self, addr):
		"""Disassemble the given address."""
		return self.board.driver.core_disasm(self.core, addr)

	def set_break(self, addr):
		"""Set a break-point at provided address."""
		#print(f"DEBUG:Py: set_break {addr:08x}")
		self.board.driver.set_break(self.core, addr)

	def clear_break(self, addr):
		"""Clear a break-point at provided address."""
		#print(f"DEBUG:Py: clear_break {addr:08x}")
		self.board.driver.clear_break(self.core, addr)


class IOComponent(Component):
	"""Represents an IO component."""

	def __init__(self, board, inst):
		Component.__init__(self, board, inst)
		board.io_components.append(self)

	def install(self, canvas):
		"""Called at UI start time to let the IO component install itself
		in the canvas. The default implementation does nothing."""
		pass

	def update(self, ress, state):
		"""Called each time the display needs to be updated for the provided
		ressource with the provided state. The default implementation does
		nothing."""
		pass

	def do_input(self, ress, state):
		"""Perform an input operation on the component."""
		self.board.driver.do_input(self.board.board, self.get_id(), ress, state)


COMPONENTS = {
	CSIM_SIMPLE: Component,
	CSIM_CORE: Core,
	CSIM_IO: Component
}

class Board:

	def __init__(self, board_path, bin_path=None, driver=None):
		self.board_path = board_path
		self.bin_path = bin_path
		self.components = []
		self.io_components = []
		self.map = {}
		if driver:
			self.driver = driver
		else:
			self.driver = lib.Driver()

		# load the board
		self.board = self.driver.load_board(board_path)
		if self.board is None:
			raise BoardError(f"cannot open board {board_path}")

		# initialize all
		self.core = None
		self.clock = None
		self.confs = None
		self.name = None

		# build the list of components
		for inst in self.driver.get_insts(self.board):
			comp = self.driver.get_comp(inst)
			comp_info = self.driver.component_info(comp)
			ctype = comp_info[1]
			py_inst = COMPONENTS[ctype](self,  inst)
			self.components.append(py_inst)
			self.map[py_inst.get_id()] = py_inst
			if isinstance(py_inst, Core):
				if self.core != None:
					raise BoardError("several cores defined!")
				else:
					self.core = py_inst

		# check for cores
		if self.core is None:
			raise BoardError("no core defined!")

		# if required, load the binary
		if bin_path is not None:
			self.load_bin(bin_path)

	def get_confs(self):
		"""Get the configuration of the board."""
		if self.confs is None:
			self.confs = Confs(self.driver.board_confs(self.board))
		return self.confs

	def run(self, time = 10):
		return self.driver.run(self.board, time)

	def step(self):
		self.driver.step(self.board)

	def get_core(self):
		"""Get the execution core."""
		return self.core

	def get_components(self):
		"""Get the components of the core."""
		return self.components

	def get_clock(self):
		"""Get the clock of the board."""
		if self.clock is None:
			self.clock = self.driver.get_clock(self.board)
		return self.clock

	def load_bin(self, path):
		"""Load the binary. Raise BoardError in case of error."""
		self.bin_path = path
		self.core.load(path)

	def parse_port(self, text):
		both = text.split('.')
		if len(both) != 2:
			raise BoardError("port format must be COMPONENT.BOARD!")
		found_inst = None
		for comp in self.components:
			if comp.name == both[0]:
				found_inst = comp
				break
		if found_inst is None:
			raise BoardError("cannot find instance '%s'!" % both[0])
		port = self.driver.find_port(found_inst.comp, both[1])
		if port == None:
			raise BoardError("cannot find port '%s' in '%s'" % (both[1], both[0]))
		return (found_inst.inst, port)

	def reset(self):
		"""Reset the state of the simulator."""
		self.driver.reset_board(self.board)
		if self.bin_path is not None:
			self.load_bin(self.bin_path)

	def release(self):
		"""Release resources used by the board."""
		self.driver.delete_board(self.board)
		self.board = None

	def get_pc(self):
		"""Get the current address of the PC."""
		return self.core.pc()

	def inst_size(self):
		"""Get the size of the current instruction. """
		return self.core.inst_size()

	def get_date(self):
		"""Get the date of the simulated board."""
		return self.driver.get_date(self.board)

	def byte_at(self, addr):
		"""Get the byte at provided address."""
		return self.driver.byte_at(self.board, addr)

	def half_at(self, addr):
		"""Get the half-word at provided address."""
		return self.driver.half_at(self.board, addr)

	def word_at(self, addr):
		"""Get the word at provided address."""
		return self.driver.word_at(self.board, addr)

	def set_log_level(self, level):
		"""Set the log level (one of CSIM_DEBUG, CSIM_INFO, etc)."""
		self.driver.set_log_level(self.board, level)

	def update_input(self):
		"""Update input components."""
		states = self.driver.flush_iostates(self.board)
		for (id, ress, state) in states:
			self.map[id].update(ress, state)

	def get_name(self):
		"""Get the name of the board."""
		if self.name is None:
			self.name = self.driver.board_name(self.board)
		return self.name

	def __str__(self):
		return f"Board {self.get_name()}"


