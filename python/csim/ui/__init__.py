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

"""UI for CSim"""

import re
import sys

from orchid import Buffer
from orchid import svg

import csim
from csim.ui.stack import Error, StackMachine, Value
import libcsim

size_re = re.compile(r"viewBox=\"[0-9.]+\s+[0-9.]\s+([0-9.]+)\s+([0-9.]+)\"")

class Image:

	def __init__(self, content, w, h):
		self.content = content
		self.w = w
		self.h = h

# SVG
def load_svg(path):
	buf = Buffer()
	size = (1, 1)

	def look_size(line):
		pat = size_re.search(line)
		if pat:
			try:
				w = float(pat.group(1))
				h = float(pat.group(2))
				return (w, h)
			except ValueError:
				pass
		return size

	with open(path) as input:
		l = input.readline()
		while not l.startswith("<svg"):
			l = input.readline()
		size = look_size(l)
		while not l.endswith(">\n"):
			l = input.readline()
			size = look_size(l)
		l = input.readline()
		while not l.startswith("</svg>"):
			buf.write(l)
			l = input.readline()
	return Image(str(buf), size[0], size[1])


def make_io(board, inst):
	"""Ensures translation of IO components"""
	comp = libcsim.get_comp(inst)
	name = libcsim.component_info(comp)[0]
	try:
		mod = __import__(f"csim.ui.{name}", fromlist=["csim.ui"])
	except ImportError as e:
		raise csim.BoardError("cannot load %s: %s" % (name, e))
	return mod.Component(board, inst)


csim.COMPONENTS[csim.CSIM_IO] = make_io


class Component(csim.IOComponent):
	"""A component ready to be displayed in Orchid."""

	POSITIONS = None

	@staticmethod
	def _init_pos():
		if Component.POSITIONS is None:

			def _right_of(display, comp, rel):
				(x, y) = rel.get_pos()
				(w, h) = rel.get_size()
				return (x + w + display.get_xspace(), y)

			def _left_of(display, comp, rel):
				(x, y) = rel.get_pos()
				(w, h) = comp.get_size()
				return (x - w - display.get_xspace(), y)

			def _below_of(display, comp, rel):
				(x, y) = rel.get_pos()
				(w, h) = rel.get_size()
				return (x, y + h + display.get_yspace())

			def _above_of(display, comp, rel):
				(x, y) = rel.get_pos()
				(w, h) = comp.get_size()
				return (x, y - h - display.get_yspace())

			Component.POSITIONS = {
				"right-of": _right_of,
				"left-of": _left_of,
				"below-of": _below_of,
				"above-of": _above_of
			}

	def _get_pos(self, display, pos):
		"""Convert pos attribute to actual position."""
		args = pos.split()
		if not args:
			return (0, 0)
		rel = display.find(args[-1])
		if rel is None:
			return (0, 0)
		key = "-".join(args[0:-1]).lower()
		Component._init_pos()
		try:
			return Component.POSITIONS[key](display, self, rel)
		except KeyError:
			return (0, 0)

	def __init__(self, board, inst):
		csim.IOComponent.__init__(self, board, inst)
		self.x = self.get_confs().get_int("x", None)
		self.y = self.get_confs().get_int("y", None)

	def get_pos(self):
		"""Get the position (x, y) of the component."""
		return (self.x, self.y)

	def get_size(self):
		"""Get the size (width, height) of the component."""
		return (1, 1)

	def map(self, display):
		"""Called to make the component to display on the canavas."""
		if self.x is None or self.y is None:
			pos = self.get_confs().get("pos")
			if pos is None:
				pos = (0, 0)
			else:
				pos = self._get_pos(display, pos)
			if self.x is None:
				self.x = pos[0]
			if self.y is None:
				self.y = pos[1]


class Point(Value):

	def __init__(self, x, y):
		self.x = x
		self.y = y

	def add(self, x):
		if isinstance(x, Point):
			return Point(self.x + x.x, self.y + x.y)
		else:
			return Point(self.x + x.as_int(), self.y + x.as_int())

	def sub(self, x):
		if isinstance(x, Point):
			return Point(self.x - x.x, self.y - x.y)
		else:
			return Point(self.x - x.as_int(), self.y - x.as_int())

	def as_str(self):
		return f"({self.x}, {self.y})"


class Color(Value):

	def __init__(self, color):
		self.color = color

	def as_str(self):
		return self.color

	def __str__(self):
		return self.color


class CompValue(Value):

	def __init__(self, comp):
		self.comp = comp

	def as_str(self):
		return f"${self.comp.get_name()}"


class DecoMachine(StackMachine):
	"""Machine for generating decoration."""

	def _do_point(self, mach):
		y = mach.pop()
		x = mach.pop()
		mach.push(Point(x, y))

	def _do_line(self, mach):
		p2 = mach.pop()
		p1 = mach.pop()
		args = {}
		if self.stroke:
			args["stroke"] = self.stroke
		self.display.line(p1.x, p1.y, p2.x, p2.y, **args)

	def _do_text(self, mach):
		t = mach.pop().as_str()
		p = mach.pop()
		args = {}
		x = p.x
		y = p.y
		if self.fill:
			args["fill"] = self.fill
		if self.anchor:
			if self.anchor.as_str() == "top":
				anchor = "middle"
				y += self.font_size
			elif self.anchor.as_str() == "bottom":
				anchor = "middle"
			else:
				anchor = self.anchor
				y += self.font_size/2
			args["text-anchor"] = anchor
		args["font-size"] = f"{self.font_size}px";
		self.display.text(x, y, t, **args)

	def _do_fill(self, mach):
		self.fill = mach.pop()

	def _do_stroke(self, mach):
		self.stroke = mach.pop()

	def _do_anchor(self, mach):
		self.anchor = mach.pop()
		if self.anchor == "none":
			self.anchor = None

	def get_comp(self, cmd, mach):
		comp = self.display.find(cmd[1:])
		if comp is None:
			raise Error(f"cannot find component {cmd}")
		else:
			mach.push(CompValue(comp))

	def _do_right_of(self, mach):
		val = mach.pop_check(CompValue)
		(x, y) = val.comp.get_pos()
		(w, h) = val.comp.get_size()
		mach.push(Point(x + w + self.display.get_xspace(), y + h/2))

	def _do_left_of(self, mach):
		val = mach.pop_check(CompValue)
		(x, y) = val.comp.get_pos()
		(w, h) = val.comp.get_size()
		mach.push(Point(x + - self.display.get_xspace(), y + h/2))

	def _do_below_of(self, mach):
		val = mach.pop_check(CompValue)
		(x, y) = val.comp.get_pos()
		(w, h) = val.comp.get_size()
		mach.push(Point(x + w/2, y + h + self.display.get_yspace()))

	def _do_above_of(self, mach):
		val = mach.pop_check(CompValue)
		(x, y) = val.comp.get_pos()
		(w, h) = val.comp.get_size()
		mach.push(Point(x + w/2, y - self.display.get_yspace()))

	def _do_font_size(self, mach):
		self.font_size = mach.pop().as_int()

	def __init__(self, display):

		# call super
		StackMachine.__init__(
			self,
			{ },
			lexer = {
				'#': lambda cmd, mach: mach.push(Color(cmd)),
				'$': self.get_comp
			}
		)
		self.display = display
		self.stroke = "black"
		self.fill = "black"
		self.anchor = None
		self.font_size = 12


class Display(svg.Canvas):
	"""Orchid component to display simulated IO components."""

	def __init__(self):
		svg.Canvas.__init__(self)
		self.xspace = 8
		self.yspace = 8
		self.map = {}

	def get_xspace(self):
		return self.xspace

	def get_yspace(self):
		return self.yspace

	def find(self, name):
		"""Find a component by its name. Return found component or None."""
		try:
			return self.map[name]
		except KeyError:
			return None

	def install(self, board):
		"""Add IO components from the board.
		May rise csim.ui.stack.Error if a decoration cannot execute."""

		# set configuration
		self.xspace = board.get_confs().get_int("xspace", self.xspace)
		self.yspace = board.get_confs().get_int("yspace", self.yspace)

		# map the components
		for io in board.io_components:
			if isinstance(io, Component):
				io.map(self)
				self.map[io.get_name()] = io

		# draw deco if any
		deco = board.get_confs().get("deco")
		if deco:
			mach = DecoMachine(self)
			mach.run(deco)

		# display them
		for io in board.io_components:
			io.install(self)

