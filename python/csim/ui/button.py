
import os.path

import csim
from csim.ui import load_svg
import libcsim

from orchid.svg import Content

class Component(csim.IOComponent):
	IMAGE = None

	def __init__(self, board, inst):
		csim.IOComponent.__init__(self, board, inst)
		confs = self.get_confs()
		self.x = confs.get_int("x", 0)
		self.y = confs.get_int("y", 0)

	def on_push(self):
		self.canvas.get_page().set_direct_attr(
			"%s-push" % self.content.get_id(), "fill", "#888888")
		#libcsim.set_state(self.inst, [1])
		self.do_input(1, 1)

	def on_release(self):
		self.canvas.get_page().set_direct_attr(
			"%s-push" % self.content.get_id(), "fill", "#000000")
		#libcsim.set_state(self.inst, [0])
		self.do_input(1, 0)

	def install(self, canvas):
		self.canvas = canvas

		# load image
		if Component.IMAGE == None:
			Component.IMAGE = load_svg(os.path.join(os.path.dirname(__file__), "button.svg"))

		# build the UI
		self.content = canvas.content(Component.IMAGE)
		self.content.scale(2)
		self.content.translate(self.x, self.y)
		self.content.add_event("onmousedown", self.on_push)
		self.content.add_event("onmouseup", self.on_release)
