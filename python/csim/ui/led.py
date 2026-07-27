"""LED component implementation."""

import os.path

import csim
from csim import ui
import libcsim

from orchid import *
from orchid.svg import Canvas, Content
from orchid.util import Buffer

class LED(Content):
	IMAGE = None

	def  __init__(self, x, y, color, **args):
		if LED.IMAGE == None:
			LED.IMAGE = ui.load_svg(os.path.join(os.path.dirname(__file__), "newled.svg"))
		Content.__init__(self, LED.IMAGE.content, **args)
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

	def __init__(self, board, inst):
		ui.Component.__init__(self, board, inst)
		confs = self.get_confs()
		self.color = confs.get("color", "#FF0000")

	def install(self, canvas):
		(x, y) = self.get_pos()
		self.shape = LED(x, y, self.color)
		canvas.record(self.shape)

	def update(self, ress, state):
		if state != self.shape.state:
			self.shape.invert()

	def get_size(self):
		return (LED.IMAGE.w, LED.IMAGE.H)
