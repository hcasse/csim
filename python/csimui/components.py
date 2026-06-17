"""Components of csimui"""

from enum import IntEnum

import csim
from csimui.util import BoardError
import yaml
from csimui import util

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
		(name, offset, size, count, stride, flags, type) = csim.register_info(self.reg)
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
		return csim.get_register_val(self.comp.inst, self.reg, i)

	def set_value(self, i, x):
		"""Set the value of the register."""
		return csim.set_register_val(self.comp.inst, self.reg, i, x)

	def make_name(self, index):
		"""Build the name of an instance of the register."""
		return csim.register_make_name(self.comp.inst, self.reg, index)


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
			(name, type, version, reg_cnt, port_cnt, size) = csim.component_info(self.comp)
			self.registers = []
			for i in range(reg_cnt):
				reg = csim.get_register(self.comp, i)
				self.registers.append(Register(self, reg))
		return self.registers


class Core(Component):
	"""Represents a core component."""

	def __init__(self, board, name, comp, inst, atts):
		Component.__init__(self, board, name, comp, inst, atts)
		self.core = csim.get_core(board.board)
		assert self.core is not None

	def load(self, path):
		"""Load the binary from the path.
		Raises BoardError if there is an error."""
		res = csim.core_load(self.core, path)
		if res != 0:
			raise BoardError(f'cannot load "{path}"')

	def pc(self):
		"""Get the current PC."""
		return csim.core_pc(self.core)

	def inst_size(self):
		"""Get the size of the current instruction."""
		return csim.core_inst_size(self.core)

	def disasm(self, addr):
		"""Disassemble the given address."""
		return csim.core_disasm(self.core, addr)

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

def make_io(board, name, comp, inst, atts):
	type = util.get(atts, "type", None)
	assert type is not None
	try:
		mod = __import__("csimui.%s" % type, fromlist=["csimui"])
	except ImportError as e:
		raise util.BoardError("cannot load %s: %s" % (type, e))
	return mod.Component(board, name, comp, inst, atts)

COMPONENTS = {
	CSIM_SIMPLE: Component,
	CSIM_CORE: Core,
	CSIM_IO: make_io
}

class Board:

	def run(self, time = 10):
		csim.run(self.board, time)

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
		board_name = util.get(desc, "name", "no name")

		# build the board
		self.board = csim.new_board(board_name, None)
		self.core = None
		self.clock = util.get(desc, "clock", 1000)
		self.quantum = util.get(desc, "quantum", 100)
		if self.clock // self.quantum != self.clock / self.quantum:
			util.warn("quantum (%d) must be a divider of master clock(%dHz)" % (self.quantum, self.clock))
		csim.set_master_clock(self.board, self.clock)

		# build the components
		comps = util.obtain(desc, "components", "no component defined")
		for (name, cdesc) in comps.items():
			type = util.obtain(cdesc, "type", "no type defined for %s" % name)
			comp = csim.find_component(type)
			if comp is None:
				raise BoardError("cannot find component %s" % type)
			info = csim.component_info(comp)
			base = int(util.get(cdesc, "base", "0"), 16)
			inst = csim.new_component(self.board, comp, name, base)
			ctype = info[1]
			obj = COMPONENTS[ctype](self, name, comp, inst, cdesc)
			self.components.append(obj)
			if isinstance(obj, Core):
				if self.core != None:
					raise BoardError("several cores defined!")
				else:
					self.core = obj

		# build the connections
		cons = util.get(desc, "connect")
		if cons != None:
			for con in cons:
				from_ = util.obtain(con, "from", "no 'from' in connection")
				(from_inst, from_port) = self.parse_port(from_)
				to_ = util.obtain(con, "to", "no 'to' in connection")
				(to_inst, to_port) = self.parse_port(to_)
				csim.connect(from_inst, from_port, to_inst, to_port)

		# check for cores
		if self.core is None:
			raise util.BoardError("no core defined!")

		# if required, load the binary
		if bin_path is not None:
			self.load_bin(bin_path)

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
			raise util.BoardError("port format must be COMPONENT.BOARD!")
		found_inst = None
		for comp in self.components:
			if comp.name == both[0]:
				found_inst = comp
				break
		if found_inst is None:
			raise util.BoardError("cannot find instance '%s'!" % both[0])
		port = csim.find_port(found_inst.comp, both[1])
		if port == None:
			raise util.BoardError("cannot find port '%s' in '%s'" % (both[1], both[0]))
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
