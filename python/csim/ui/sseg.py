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

"""7-segment display"""


import os.path

import csim
from csim import ui
import libcsim

from orchid.svg import Content

class SSegShape(Content):
	SCALE = .5
	SEGS = [
		"up",
		"upleft",
		"upright",
		"mid",
		"downleft",
		"downright",
		"down"
	]

	def  __init__(self, image, x, y, on, off, **args):
		Content.__init__(self, image.content, **args)
		self.on = on
		self.off = off
		self.scale(self.SCALE)
		self.translate(x, y)

	def paint(self, state):
		"""Paint the cells to the right colors."""
		for i in range(7):
			if ((state >> i) & 1) != 0:
				color = self.on
			else:
				color = self.off
			self.set_direct_attr(
				f"{self.get_id()}{self.SEGS[i]}",
				"style",
				f"fill:{color}")


class Component(ui.Component):
	IMAGE = None

	def __init__(self, board, inst):
		ui.Component.__init__(self, board, inst)
		self.state = 0
		self.on = "red"
		self.off = "#444444"

	def map(self, display):
		if Component.IMAGE == None:
			Component.IMAGE = ui.load_svg(os.path.join(os.path.dirname(__file__), "7segments.svg"))
		ui.Component.map(self, display)

	def install(self, canvas):
		self.canvas = canvas
		(x, y) = self.get_pos()
		self.shape = SSegShape(self.IMAGE, x, y, self.on, self.off)
		canvas.record(self.shape)

	def get_size(self):
		return (
			self.IMAGE.w * SSegShape.SCALE,
			self.IMAGE.h * SSegShape.SCALE
		)

	def update(self, ress, state):
		if state != self.state:
			self.state = state
			#if self.parent.online():
			self.shape.paint(state)
