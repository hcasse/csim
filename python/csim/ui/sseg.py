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

class Component(ui.Component):
	IMAGE = None
	SEGS = [
		"up",
		"upleft",
		"upright",
		"mid",
		"downleft",
		"downright",
		"down"
	]

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
		self.content = canvas.content(Component.IMAGE.content)
		self.content.scale(1)
		(x, y) = self.get_pos()
		self.content.translate(x, y)

	def get_size(self):
		return (self.IMAGE.w * 2, self.IMAGE.h * 2)

	def paint(self):
		"""Paint the cells to the right colors."""
		for i in range(7):
			if ((state >> i) & 1) != 0:
				color = self.on
			else:
				color.self.off
			self.set_direct_attr(
				f"{self.content.get_id()}{self.SEGS[i]}",
				"style",
				f"fill:{color}")

	def update(self, ress, state):
		if state != self.shape.state:
			self.state = state
			if self.parent.online():
				self.paint()
