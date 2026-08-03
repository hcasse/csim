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

"""LED component implementation."""

import os.path

import csim
from csim import ui
import libcsim

from orchid import *
from orchid.svg import Canvas, Content
from orchid.util import Buffer

class LED(Content):

	def  __init__(self, image, x, y, color, **args):
		Content.__init__(self, image.content, **args)
		self.color = color
		self.state = False
		#self.scale(.1)
		self.scale(.2)
		self.translate(x, y)

	def set(self, color1, color2):
		if self.parent.online():
			id = self.get_id()
			self.set_direct_attr(id + "-back", "style", "fill:" + color1)

	def paint(self):
		if self.state:
			self.set(self.color, self.color)
		else:
			self.set("#CCCCCC", "#CCCCCC")

	def on(self):
		if not self.state:
			self.state = True
			self.paint()

	def off(self):
		if self.state:
			#self.set("#CCCCCC", "#FFFFFF")
			self.state = False
			self.paint()

	def invert(self):
		if self.state:
			self.off()
		else:
			self.on()


class Component(ui.Component):
	IMAGE = None

	def __init__(self, board, inst):
		ui.Component.__init__(self, board, inst)
		confs = self.get_confs()
		self.color = confs.get("color", "#FF0000")

	def install(self, canvas):
		(x, y) = self.get_pos()
		self.shape = LED(Component.IMAGE, x, y, self.color)
		canvas.record(self.shape)

	def update(self, ress, state):
		if state != self.shape.state:
			self.shape.invert()

	def get_size(self):
		return (self.IMAGE.w * .2, self.IMAGE.h * .2)

	def map(self, display):
		if Component.IMAGE is None:
			Component.IMAGE = ui.load_svg(os.path.join(os.path.dirname(__file__), "newled.svg"))
		ui.Component.map(self, display)
