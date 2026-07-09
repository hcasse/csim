"""LED component implementation."""

import os.path

import csim
from csim.ui import load_svg
import libcsim

from orchid import *
from orchid.svg import Canvas, Content
from orchid.util import Buffer

class LED(Content):
	IMAGE = None

	def  __init__(self, x, y, color, **args):
		if LED.IMAGE == None:
			LED.IMAGE = load_svg(os.path.join(os.path.dirname(__file__), "newled.svg"))
		Content.__init__(self, LED.IMAGE, **args)
		self.color = color
		self.state = False
		#self.scale(.1)
		self.scale(.2)
		self.translate(x, y)

	def set(self, color1, color2):
		if self.parent.online():
			id = self.get_id()
			#self.set_direct_attr(id + "_path1", "fill", color1)
			#self.set_direct_attr(id + "_path2", "fill", color1)
			#self.set_direct_attr(id + "_stop1", "style", "stop-color:" + color2)
			#self.set_direct_attr(id + "_stop2", "style", "stop-color:" + color2 + ";stop-opacity:0")
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


class Component(csim.IOComponent):

	def __init__(self, board, name, comp, inst, atts):
		csim.IOComponent.__init__(self, board, name, comp, inst, atts)
		self.x = int(csim.get(atts, "x", 0))
		self.y = int(csim.get(atts, "y", 0))
		self.color = csim.get(atts, "color", "#FF0000")

	def install(self, canvas):
		self.shape = LED(self.x, self.y, self.color)
		canvas.record(self.shape)

	def update(self, ress, state):
		if state != self.shape.state:
			self.shape.invert()
