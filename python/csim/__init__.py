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
import yaml

import libcsim

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


def get(map, name, default = None):
	try:
		return map[name]
	except KeyError:
		return default

def obtain(map, name, msg):
	try:
		return map[name]
	except KeyError:
		raise BoardError(msg)


CSIM_SIMPLE = 1
CSIM_CORE = 2
CSIM_IO = 3

class RType(IntEnum):
	NONE = 0
	BITS = 1
	INT = 2
	ADDR = 3
	FLOAT32 = 4
	FLOAT64 = 5

class Register:
	"""Representation of a register."""

	def __init__(self, comp, reg):
		self.comp = comp
		self.reg = reg
		self.name = None
		self.offset = None
		self.size = None
		self.count = None
		self.stride = None
		self.flags = None
		self.type = None

	def fill(self):
		(name, offset, size, count, stride, flags, type) = libcsim.register_info(self.reg)
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
		return libcsim.get_register_val(self.comp.inst, self.reg, i)

	def set_value(self, i, x):
		"""Set the value of the register."""
		return libcsim.set_register_val(self.comp.inst, self.reg, i, x)

	def make_name(self, index):
		"""Build the name of an instance of the register."""
		return libcsim.register_make_name(self.comp.inst, self.reg, index)


class Component:
	"""Represents a simple component."""

	def __init__(self, board, name, comp, inst, atts):
		self.board = board
		self.name = name
		self.comp = comp
		self.inst = inst
		self.registers = None

	def get_name(self):
		"""Get the name of the component."""
		return self.name

	def get_registers(self):
		"""Get registers of the component. List of Register objects."""
		if not self.registers:
			(name, type, version, reg_cnt, port_cnt, size) = libcsim.component_info(self.comp)
			self.registers = []
			for i in range(reg_cnt):
				reg = libcsim.get_register(self.comp, i)
				self.registers.append(Register(self, reg))
		return self.registers


class Core(Component):
	"""Represents a core component."""

	def __init__(self, board, name, comp, inst, atts):
		Component.__init__(self, board, name, comp, inst, atts)
		self.core = libcsim.get_core(board.board)
		assert self.core

	def load(self, path):
		"""Load the binary from the path.
		Raises BoardError if there is an error."""
		res = libcsim.core_load(self.core, path)
		if res != 0:
			raise BoardError(f'cannot load "{path}"')

	def pc(self):
		"""Get the current PC."""
		return libcsim.core_pc(self.core)

	def inst_size(self):
		"""Get the size of the current instruction."""
		return libcsim.core_inst_size(self.core)

	def disasm(self, addr):
		"""Disassemble the given address."""
		return libcsim.core_disasm(self.core, addr)

class IOComponent(Component):
	"""Represents an IO component."""

	def __init__(self, board, name, comp, inst, atts):
		Component.__init__(self, board, name, comp, inst, atts)
		board.io_components.append(self)

	def install(self, canvas):
		"""Called at UI start time to let the IO component install itself
		in the canvas. The default implementation does nothing."""
		pass

	def update(self):
		"""Called each time the display needs to be updated for the
		current component."""
		pass

COMPONENTS = {
	CSIM_SIMPLE: Component,
	CSIM_CORE: Core,
	CSIM_IO: Component
}

class Board:

	def __init__(self, board_path, bin_path=None):
		self.board_path = board_path
		self.bin_path = bin_path
		self.components = []
		self.io_components = []

		# load the board
		try:
			with open(board_path, "r") as input:
				desc = yaml.safe_load(input)
				if desc is None:
					raise BoardError(f"empty board in {board_path}")
		except OSError as exn:
			raise BoardError(str(exn))
		board_name = get(desc, "name", "no name")

		# build the board
		self.board = libcsim.new_board(board_name)
		self.core = None
		self.clock = get(desc, "clock", 1000)
		self.quantum = get(desc, "quantum", 100)
		if self.clock // self.quantum != self.clock / self.quantum:
			warn("quantum (%d) must be a divider of master clock(%dHz)" % (self.quantum, self.clock))
		libcsim.set_master_clock(self.board, self.clock)

		# build the components
		comps = obtain(desc, "components", "no component defined")
		for (name, cdesc) in comps.items():
			type = obtain(cdesc, "type", "no type defined for %s" % name)
			comp = libcsim.find_component(type)
			if comp is None:
				raise BoardError("cannot find component %s" % type)
			info = libcsim.component_info(comp)
			base = int(get(cdesc, "base", "0"), 16)
			inst = libcsim.new_component(self.board, comp, name, base)
			ctype = info[1]
			obj = COMPONENTS[ctype](self, name, comp, inst, cdesc)
			self.components.append(obj)
			if isinstance(obj, Core):
				if self.core != None:
					raise BoardError("several cores defined!")
				else:
					self.core = obj

		# build the connections
		cons = get(desc, "connect")
		if cons != None:
			for con in cons:
				from_ = obtain(con, "from", "no 'from' in connection")
				(from_inst, from_port) = self.parse_port(from_)
				to_ = obtain(con, "to", "no 'to' in connection")
				(to_inst, to_port) = self.parse_port(to_)
				libcsim.connect(from_inst, from_port, to_inst, to_port)

		# check for cores
		if self.core is None:
			raise BoardError("no core defined!")

		# if required, load the binary
		if bin_path is not None:
			self.load_bin(bin_path)

	def run(self, time = 10):
		libcsim.run(self.board, time)

	def get_core(self):
		"""Get the execution core."""
		return self.core

	def get_components(self):
		"""Get the components of the core."""
		return self.components

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
		port = libcsim.find_port(found_inst.comp, both[1])
		if port == None:
			raise BoardError("cannot find port '%s' in '%s'" % (both[1], both[0]))
		return (found_inst.inst, port)

	def update(self):
		"""Called each the IO components needs to be updated."""
		for comp in self.io_components:
			comp.update()

	def reset(self):
		"""Reset the state of the simulator."""
		csim.reset_board(self.board)
		if self.bin_path is not None:
			self.load_bin(self.bin_path)

	def release(self):
		"""Release resources used by the board."""
		csim.delete_board(self.board)
		self.board = None

	def get_pc(self):
		"""Get the current address of the PC."""
		return self.core.pc()

	def inst_size(self):
		"""Get the size of the current instruction. """
		return self.core.inst_size()

	def get_date(self):
		"""Get the date of the simulated board."""
		return csim.get_date(self.board)

	def byte_at(self, addr):
		"""Get the byte at provided address."""
		return csim.byte_at(self.board, addr)

	def half_at(self, addr):
		"""Get the half-word at provided address."""
		return csim.half_at(self.board, addr)

	def word_at(self, addr):
		"""Get the word at provided address."""
		return csim.word_at(self.board, addr)

