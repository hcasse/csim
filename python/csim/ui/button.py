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

"""Button component implementation."""

import os.path

import csim
from csim import ui
import libcsim

from orchid.svg import Content

class Component(ui.Component):
	IMAGE = None

	def __init__(self, board, inst):
		ui.Component.__init__(self, board, inst)

	def on_push(self):
		self.canvas.get_page().set_direct_attr(
			"%s-push" % self.content.get_id(), "fill", "#888888")
		self.do_input(1, 1)

	def on_release(self):
		self.canvas.get_page().set_direct_attr(
			"%s-push" % self.content.get_id(), "fill", "#000000")
		self.do_input(1, 0)

	def map(self, display):
		if Component.IMAGE == None:
			Component.IMAGE = ui.load_svg(os.path.join(os.path.dirname(__file__), "button.svg"))
		ui.Component.map(self, display)

	def install(self, canvas):
		self.canvas = canvas
		self.content = canvas.content(Component.IMAGE.content)
		self.content.scale(2)
		(x, y) = self.get_pos()
		self.content.translate(x, y)
		self.content.add_event("onmousedown", self.on_push)
		self.content.add_event("onmouseup", self.on_release)

	def get_size(self):
		return (self.IMAGE.w * 2, self.IMAGE.h * 2)
