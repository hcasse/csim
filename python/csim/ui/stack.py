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

"""Stack machine for decoration interpretation."""

class Error(Exception):
	"""Raised if there is an error during execution."""

	def __init__(self, msg):
		self.msg = msg

	def __str__(self):
		return self.msg


class Value:

	def as_int(self):
		return 0

	def as_str(self):
		return ""

	def add(self, x):
		return Value()

	def sub(self, x):
		return Value()


class Int(Value):

	def __init__(self, n):
		self.n = n

	def as_int(self):
		return self.n

	def as_str(self):
		return str(self.n)

	def add(self, x):
		return Int(self.n + x.as_int())

	def sub(self, x):
		return Int(self.n - x.as_int())


class Str(Value):

	def __init__(self, s):
		self.s = s

	def as_str(self):
		return self.s

	def as_int(self):
		try:
			return int(self.s)
		except KeyError:
			return 0

	def add(self, x):
		return Str(self.s + x.as_str())

	def __str__(self):
		return self.s


class StackMachine:
	"""Stack machine to interpret decoration."""

	def start_str(self, cmd, mach):
		if cmd.endswith('"'):
			cmd = cmd[0:-1]
		else:
			self.in_str = True
		mach.push(Str(cmd[1:]))

	def process_str(self, cmd, mach):
		if cmd.endswith('"'):
			self.in_str = False
			cmd = cmd[0:-1]
		mach.push(Str(mach.pop().as_str() + " " + cmd))

	def parse_int(self, cmd, mach):
		try:
			mach.push(int(cmd))
		except ValueError:
			mach.push(Int(0))

	@staticmethod
	def add(mach):
		y = mach.pop()
		x = mach.pop()
		mach.push(x.add(y))

	@staticmethod
	def sub(mach):
		y = mach.pop()
		x = mach.pop()
		mach.push(x.sub(y))

	@staticmethod
	def set(mach):
		val = mach.pop()
		key = mach.pop().as_str()
		mach.define(key, lambda mach: val)

	def __init__(self, map = None, lexer = None):

		# define function map
		self.map = {
			"+": 	StackMachine.add,
			"-": 	StackMachine.sub,
			"set":	StackMachine.set
		}
		if map is not None:
			self.map = self.map | map

		# define lexer map
		self.lexer = {
			'"': self.start_str
		}
		for c in "0123456789":
			self.lexer[c] = self.parse_int
		if lexer is not None:
			self.lexer = self.lexer | lexer

		self.stack = []
		self.in_str = False

	def define(self, key, fun):
		self.map[key] = fun

	def pop(self):
		if not self.stack:
			raise Error("empty stack")
		return self.stack.pop()

	def pop_check(self, cls):
		val = self.pop()
		if not isinstance(val, cls):
			raise Error(f"Argument should be {cls}!")
		return val

	def push(self, val):
		self.stack.append(val)

	def run(self, prog):
		"""Execute the program.
		Raise Error if there is an error."""
		cmds = prog.strip().split()

		for cmd in cmds:

			# in string
			if self.in_str:
				self.process_str(cmd)

			# other
			#print(f"command {cmd}")
			try:
				self.lexer[cmd[0]](cmd, self)
			except KeyError:
				try:
					self.map[cmd](self)
				except KeyError:
					raise Error(f"unknown command {cmd}")

	def make_int(self, n):
		return Int(n)

	def make_str(self, s):
		return Str(s)






