"""LED component implementation."""

import os.path
from orchid import *
from orchid.svg import Canvas, Content
from orchid.util import Buffer
from csimui import components
from csimui.util import *
import csim


class LED(Content):
	IMAGE = None

	def  __init__(self, x, y, color, **args):
		if LED.IMAGE == None:
			LED.IMAGE = load_svg(os.path.join(os.path.dirname(__file__), "led.svg"))
		Content.__init__(self, LED.IMAGE, **args)
		self.color = color
		self.state = False
		self.scale(.1)
		self.translate(x, y)

	def set(self, color1, color2):
		if self.parent.online():
			id = self.get_id()
			self.set_direct_attr(id + "_path1", "fill", color1)
			self.set_direct_attr(id + "_path2", "fill", color1)
			self.set_direct_attr(id + "_stop1", "style", "stop-color:" + color2)
			self.set_direct_attr(id + "_stop2", "style", "stop-color:" + color2 + ";stop-opacity:0")

	def on(self):
		if not self.state:
			self.set(self.color, self.color)
			self.state = True

	def off(self):
		if self.state:
			self.set("#CCCCCC", "#FFFFFF")
			self.state = False

	def invert(self):
		if self.state:
			self.off()
		else:
			self.on()
	

class Component(components.IOComponent):

	def __init__(self, board, name, comp, inst, atts):
		components.IOComponent.__init__(self, board, name, comp, inst, atts)
		self.x = int(get(atts, "x", 0))
		self.y = int(get(atts, "y", 0))
		self.color = get(atts, "color", "#FF0000")

	def install(self, canvas):
		self.shape = LED(self.x, self.y, self.color)
		canvas.record(self.shape)

	def update(self):
		res = csim.get_state(self.inst, 1)
		if res[0] != self.shape.state:
			self.shape.invert()
