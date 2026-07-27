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
		"""Add IO components from the board."""

		# set configuration
		self.xspace = board.get_confs().get_int("xspace", self.xspace)
		self.yspace = board.get_confs().get_int("yspace", self.yspace)

		# map the components
		for io in board.io_components:
			if isinstance(io, Component):
				io.map(self)
				self.map[io.get_name()] = io

		# display them
		for io in board.io_components:
			io.install(self)

