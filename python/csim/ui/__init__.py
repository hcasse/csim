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
