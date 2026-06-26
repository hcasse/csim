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

"""Application for csimui."""

import csim
from orchid import *
from csim.ui import Display

RUN_FREQ = 10

class MyPage(Page):

	def __init__(self, app, board):
		self.board = board
		self.date = Label("XXXXXXXXX")
		self.addr = Label("XXXX XXXXX")
		self.inst = Label("NOP")
		self.run_but = Button("Run", on_click=self.run)
		self.stop_but = Button("Stop", on_click=self.stop, enabled=False)
		self.step_but = Button("Step", on_click=self.step)
		self.display = Display()
		self.display.install(board)
		Page.__init__(
			self,
			VGroup([
				HGroup([
					Button("reset", on_click=self.reset),
					self.run_but,
					self.stop_but,
					self.step_but,
					self.date,
					self.addr,
					self.inst
				]),
				self.display
			]),
			app = app
		)
		self.show_current()
		self.timer = Timer(self, trigger=self.run_once, period=1000/RUN_FREQ)
		self.quantum = board.get_clock() // RUN_FREQ

	def show_current(self):
		addr = self.board.core.pc()
		self.addr.set_text("%08x" % addr)
		text = self.board.core.disasm(addr)
		self.inst.set_text(text)
		self.date.set_text(f"{self.board.get_date():9}")
		self.board.update_input()

	def run(self):
		self.timer.start()
		self.run_but.disable()
		self.step_but.disable()
		self.stop_but.enable()

	def stop(self):
		self.timer.stop()
		self.run_but.enable()
		self.step_but.enable()
		self.stop_but.disable()

	def step(self):
		self.board.step()
		self.show_current()

	def run_once(self):
		self.board.run(self.quantum)
		self.show_current()

	def reset(self):
		self.board.reset()
		self.show_current()


class MyApp(Application):

	def __init__(self, board):
		Application.__init__(self, "CSIM")
		self.fst = MyPage(self, board)

	def first(self):
		return self.fst
