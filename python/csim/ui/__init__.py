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

import sys
from orchid import Buffer
from orchid import svg
import csim

# SVG
def load_svg(path):
	buf = Buffer()
	with open(path) as input:
		l = input.readline()
		while not l.startswith("<svg"):
			l = input.readline()
		while not l.endswith(">\n"):
			l = input.readline()
		l = input.readline()
		while not l.startswith("</svg>"):
			buf.write(l)
			l = input.readline()
	return str(buf)


def make_io(board, name, comp, inst, atts):
	"""Ensures translation of IO components"""
	type = csim.get(atts, "type", None)
	assert type is not None
	try:
		mod = __import__("csim.ui.%s" % type, fromlist=["csim.ui"])
	except ImportError as e:
		raise csim.BoardError("cannot load %s: %s" % (type, e))
	return mod.Component(board, name, comp, inst, atts)


csim.COMPONENTS[csim.CSIM_IO] = make_io


class Display(svg.Canvas):
	"""Orchid component to display simulated IO components."""

	def __init__(self):
		svg.Canvas.__init__(self)

	def install(self, board):
		"""Add IO components from the board."""
		for io in board.io_components:
			io.install(self)
