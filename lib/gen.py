#!/usr/bin/env python3
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

"""Server/client generator for CSim."""

import argparse
import re
import sys

# parse arguments
parser = argparse.ArgumentParser("Client/server protocol generator for CSim.")

parser.add_argument("description")
parser.add_argument("output")
parser.add_argument("-v", "--verbose", action="store_true")
parser.add_argument("--python", "-p", action="store_true", help="Generate Python client file.")
parser.add_argument("-c", action="store_true", help="Generate Python server file.")
args = parser.parse_args()
VERBOSE = args.verbose

MODE_NORMAL = 0
MODE_IN_ATTR = 1
MODE_IN_COMMENT = 2

class SyntaxError(Exception):

	def __init__(self, msg):
		self.msg = msg

	def __str__(self):
		return self.msg


def say(msg):
	if VERBOSE:
		sys.stderr.write(f"{msg}\n")

def fatal(msg):
	sys.stderr.write(f"ERROR: {msg}\n")
	exit(1)

class Error:

	def __init__(self, name):
		self.name = name


class Command:

	def __init__(self, name, inputs, outputs):
		self.name = name
		self.inputs = inputs
		self.outputs = outputs
		self.attrs = None


# parse input
MULTI_RE = re.compile(r"^\s*\/\*")
SINGLE_RE = re.compile(r"^\s*\/\/")
ATTR_RE = re.compile(r"^\s*([a-zA-Z_0-9]+)\s*=")
CMD_RE = re.compile(r"^\s*([a-zA-Z_0-9]+)\s*:\s*(.*)\s*=>\s*(.*)\s*")
ERROR_RE = re.compile(r"^\s*error\s+([a-zA-Z_0-9]+)")

# types
TYPES = {
	"byte",
	"half",
	"word",
	"long",
	"string"
}

cmds = {}
errors = { "BAD_COMMAND": Error("BAD_COMMAND") }
global_attrs = None

attrs = {}
attr_name = None
attr_value = None
last_cmd = None

def count_braces(text):
	return text.count("{") - text.count("}")

def declare_attr(name, value):
	value = value.strip()
	if value.startswith("{"):
		value = value[1:]
	if value.endswith("}"):
		value = value[:-1]
	attrs[name] = value
	#print(f"DEBUG: declare {name} = {value}")

def parse_param(text):
	try:
		p = text.index(":")
		name = text[:p].strip()
		type = text[p+1:].strip()
		if type.endswith('*'):
			mult = True
			type = type[:-1]
		else:
			mult = False
		if not type in TYPES:
			raise SyntaxError(f"unknown type '{type}'")
		return (name, type, mult)
	except ValueError:
		raise SyntaxError(f"type missing in parameter '{text}'")

def parse_command(name, inputs, outputs):
	#print(f"DEBUG: command {name}: {inputs} => {outputs}")
	if not inputs:
		inputs = []
	else:
		inputs = [parse_param(param) for param in inputs.split(",")]
	if not outputs:
		outputs = []
	else:
		outputs = [parse_param(param) for param in outputs.split(",")]
	return Command(name,
		inputs,
		outputs)


with open(args.description) as input:
	mode = MODE_NORMAL
	line_num = 0

	try:

		for line in input.readlines():
			if line.endswith("\n"):
				line = line[:-1]
			line_num += 1
			#print(f"DEBUG: mode {mode}: {line_num}: {line}")

			# normal mode
			if mode == MODE_NORMAL:

				# multi-line comment
				match = MULTI_RE.match(line)
				if match:
					line = line.strip()
					if not line.endswith("*/"):
						mode = MODE_IN_COMMENT
					continue

				# attribute declaration
				match = ATTR_RE.match(line)
				if match:
					attr_name = match.group(1)
					attr_value = line[match.end():]
					cnt = count_braces(attr_value)
					if cnt == 0:
						declare_attr(attr_name, attr_value)
					else:
						mode = MODE_IN_ATTR
					continue

				# error case
				match = ERROR_RE.match(line)
				if match:
					name = match.group(1)
					if name in errors:
						fatal(f"{args.descriptio}:{line_num}: already declared error!")
					errors[name] = Error(name)
					continue

				# command declaration
				match = CMD_RE.match(line)
				if match:
					if global_attrs is None:
						global_attrs = attrs
					else:
						last_cmd.attrs = attrs
					attrs = {}
					name = match.group(1)
					if name in cmds:
						fatal(f"{args.descriptio}:{line_num}: already declared command!")
					last_cmd = parse_command(name, match.group(2), match.group(3))
					cmds[last_cmd.name] = last_cmd
					continue

				# default case
				if not match:
					line = line.strip()
					match = SINGLE_RE.match(line)
					if not match and line != "":
						fatal(f"{args.description}:{line_num}: syntax error")

			# in comment mode
			elif mode == MODE_IN_COMMENT:
				line = line.strip()
				if line.endswith("*/"):
					mode = MODE_NORMAL

			# in attribute mode
			else:
				attr_value += "\n" + line
				if count_braces(attr_value) == 0:
					declare_attr(attr_name, attr_value)
					mode = MODE_NORMAL

			#print(f"{line_num} {line}")

		if last_cmd:
			last_cmd.attrs = attrs
		else:
			global_attrs = attrs

	except SyntaxError as exn:
		print(f"{args.description}:{line_num}: {exn}")

# generate output
ESCAPE_RE = re.compile(r"\$\(([^)]*)\)")

def generate(output, text, map, python=None):
	"""Generate the template replaceing $(...) sequences by the function
	call in the map."""
	base = None
	for line in text.split('\n'):
		if not line:
			output.write('\n')
			continue
		if python is not None:
			if base == None:
				base = 0
				while line[base] == '\t':
					base += 1
			else:
				line = '\t' * python + line[base:]
		#print(f"[{line}]")
		match = ESCAPE_RE.search(line)
		while match:
			output.write(text[:match.start()])
			name = match.group(1).strip()
			p = name.find(' ')
			if p < 0:
				args = []
			else:
				args = [arg.strip() for arg in name[p+1].split(',')]
				name = name[:p]
			map[name](output, args)
			line = line[match.end():]
			match = ESCAPE_RE.search(line)
		output.write(line)
		output.write('\n')

def write_indent(output, text, indent):
	fix = None
	for line in text.split('\n'):
		if line and fix is None:
			i = 0
			for c in line:
				if c != '\t':
					break
				else:
					i += 1
			fix = max(0, indent - i)
		if fix is not None:
			output.write('\t' * fix)
		output.write(line)
		output.write('\n')

# Python generation
if args.python:

	def imports(output, args):
		output.write("""
from enum import IntEnum
import sys
import struct
import subprocess
""")

	def declarations(output, args):

		# generate commands
		if cmds:
			output.write("class _Com(IntEnum):\n")
			i = 0
			for cmd in cmds.values():
				output.write(f'\t{cmd.name.upper()} = {i}\n')
				i += 1

		# generate errors
		if errors:
			output.write("\nclass ErrorCode(IntEnum):\n")
			i = 1
			output.write(f"\tOK = 0\n")
			for error in errors.values():
				output.write(f"\t{error.name.upper()} = {i}\n")
				i += 1
			output.write("""
class Error(Exception):

	def __init__(self, code, msg=""):
		self.code = code
		self.msg = msg

		def __str__(self):
			return f"{self.code}: {semf.msg}"
""")

	def connect(output, args):
		if len(args) != 1:
			fatal("connect requires command path!")
		output.write("""
		self._server = subprocess.Popen(
			SERVER_PATH,
			stdin=subprocess.PIPE,
			stdout=subprocess.PIPE
		)
		self._input = self._server.stdout
		self._output = self._server.stdin
""")

	def methods(output, args):

		# generate my own functions
		output.write("""
	def _put_byte(self, value):
		self._output.write(struct.pack("B", value))

	def _put_half(self, value):
		self._output.write(struct.pack("H", value))

	def _put_word(self, value):
		self._output.write(struct.pack("I", value))

	def _put_long(self, value):
		self._output.write(struct.pack("Q", value))

	def _put_string(self, s):
		data = s.encode("utf-8")
		self._put_word(len(data) + 1)
		self._output.write(data)
		self._put_byte(0)

	def _get_chunk(self, size):
		result = bytearray()
		while len(result) < size:
			data = self._input.read(size - len(result))
			if not data:
				raise EOFError()
			result.extend(data)
		return bytes(result)

	def _send(self):
		self._output.flush()

	def _get_byte(self):
		return struct.unpack("B", self._get_chunk(1))[0]

	def _get_half(self):
		return struct.unpack("H", self._get_chunk(2))[0]

	def _get_word(self):
		return struct.unpack("I", self._get_chunk(4))[0]

	def _get_long(self):
		return struct.unpack("Q", self._get_chunk(8))[0]

	def _get_string(self):
		length = self._get_word()
		return self._get_chunk(length).decode("utf-8")[:-1]

""")
		# generate commands
		for cmd in cmds.values():
			args = ','.join(param[0] for param in cmd.inputs)
			if args:
				args = f", {args}"
			output.write(f'\tdef {cmd.name}(self{args}):\n')
			output.write(f"\t\tself._put_byte(_Com.{cmd.name.upper()}.value)\n")

			# generate inputs
			for (name, type, mult) in cmd.inputs:
				if not mult:
					output.write(f"\t\tself._put_{type}({name})\n")
				else:
					output.write(f"\t\tself._put_word(len({name}))\n")
					output.write(f"\t\tfor i in {name}):\n")
					output.write(f"\t\t\tself._put_{type}({name})[i]\n")
				output.write('\t\tself._send()\n')

			# process return code
			output.write(f'\t\t_code = self._get_byte()\n')
			output.write(f'\t\tif _code != ErrorCode.OK.value:\n')
			output.write(f'\t\t\traise Error(ErrorCode(_code), self._get_string())\n')

			# generate input
			for (name, type, mult) in cmd.outputs:
				if not mult:
					output.write(f"\t\t{name} = self._get_{type}()\n")
				else:
					output.write(f"\t\t{name}_cnt = self._get_word()\n")
					output.write(f"\t\t{name} = []\n")
					output.write(f"\t\tfor i in range({name}_cnt):\n")
					output.write(f"\t\t\t{name}.append(self._get_{type}())\n")

			# post-processing input
			try:
				text = cmd.attrs["python_client"]
				generate(output, text, {}, 2)
			except KeyError:
				pass

			# return result
			if cmd.outputs:
				if len(cmd.outputs) == 1:
					output.write(f'\t\treturn {cmd.outputs[0][0]}\n')
				else:
					content = ", ".join(out[0] for out in cmd.outputs)
					output.write(f'\t\treturn ({content})\n')

			output.write('\n')

	# generate
	try:
		text = global_attrs["python"]
	except KeyError:
		fatal("python attribute required for Python generation")
	with open(args.output, "w") as out:
		generate(out, text, {
			"imports": imports,
			"declarations": declarations,
			"connect": connect,
			'methods': methods
		}, python=0)


# C generation
if args.c:

	def includes(output, args):
		output.write("""
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
""")

	def defines(output, args):
		output.write("""
#define _IN	0
#define _OUT	1
#define _BUFFER_SIZE	2048
#define _VERSION	"0.1"
""")
		i = 0
		for cmd in cmds.values():
			output.write(f"#define _{cmd.name.upper()} {i}\n")
			i += 1
		output.write("#define OK	0\n")
		i = 1
		for error in errors.values():
			output.write(f"#define ERROR_{error.name.upper()} {i}\n")
			i += 1

	def declarations(output, args):
		output.write("""
uint32_t _msg_size = _BUFFER_SIZE;
uint32_t _msg_avail = 0;
uint8_t *_msg_buf, *_msg_ptr;
uint8_t _error = OK;

void _reset_msg() {
	_msg_ptr = _msg_buf;
}

void _init_msg() {
	_msg_buf = malloc(_msg_size);
	_reset_msg();
}

void _extend_msg() {
	_msg_size *= 2;
	uint32_t used = _msg_ptr - _msg_buf;
	_msg_buf = realloc(_msg_buf, _msg_size);
	_msg_ptr = _msg_buf + used;
}

void _put_chunk(void *data, int size) {
	while(_msg_ptr + size > _msg_buf + _msg_size)
		_extend_msg();
	memcpy(_msg_ptr, data, size);
	_msg_ptr += size;
}

void _put_byte(uint8_t byte) {
	_put_chunk(&byte, sizeof(byte));
}

void _put_half(uint16_t half) {
	_put_chunk(&half, sizeof(half));
}

void _put_word(uint32_t word) {
	_put_chunk(&word, sizeof(word));
}

void _put_long(uint64_t long_) {
	_put_chunk(&long_, sizeof(long_));
}

void _put_string(const char *str) {
	uint32_t size = strlen(str) + 1;
	_put_word(size);
	_put_chunk((void *)str, size);
}

void _send_msg() {
	write(_OUT, _msg_buf, _msg_ptr - _msg_buf);
}

void _receive_msg() {
	_msg_avail = read(_IN, _msg_buf, _msg_size);
	if(_msg_avail == 0)
		exit(0);
	_msg_ptr = _msg_buf;
}

void *_get_chunk(int size) {

	/* load rest of message */
	if(_msg_buf + _msg_avail < _msg_ptr + size) {

		/* require enlargement */
		if(_msg_ptr + size > _msg_ptr + _msg_size)
			_extend_msg();

		/* read missing value */
		while(_msg_buf + _msg_avail < _msg_ptr + size)
			_msg_avail += read(_IN, _msg_buf + _msg_avail, _msg_size - _msg_avail);
	}

	/* return memory */
	void *res = _msg_ptr;
	_msg_ptr += size;
	return res;
}

uint8_t _get_byte() {
	return *(uint8_t *)_get_chunk(sizeof(uint8_t));
}

uint16_t _get_half() {
	return *(uint16_t *)_get_chunk(sizeof(uint16_t));
}

uint32_t _get_word() {
	return *(uint32_t *)_get_chunk(sizeof(uint32_t));
}

uint64_t _get_long() {
	return *(uint64_t *)_get_chunk(sizeof(uint64_t));
}

const char *_get_string() {
	uint32_t size = _get_word();
	return (const char *)_get_chunk(size);
}

void raise(int code, const char *fmt, ...) {
	char buf[256];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buf, sizeof(buf), fmt, args);
	va_end(args);
	_error = code;
	_put_byte(code);
	_put_string(buf);
}

""")

	def init(output, args):
		output.write('\t_init_msg();\n')

	c_types = {
		"byte": "uint8_t",
		"half": "uint16_t",
		"word": "uint32_t",
		"long": "uint64_t",
		"string": "const char *"
	}

	def receive(output, args):

		# switch start
		output.write("""
	_receive_msg();
	uint8_t _cmd = _get_byte();
	switch(_cmd) {
""")

		# cases
		for cmd in cmds.values():
			output.write(f'\n\t\t\tcase _{cmd.name.upper()}: {{\n')

			# generate input read
			for (name, type, mult) in cmd.inputs:
				if not mult:
					output.write(f'\t\t\t\t{c_types[type]} {name} = _get_{type}();\n')
				else:
					output.write(f'\t\t\t\tint {name}_cnt = _get_word();\n')
					output.write(f'\t\t\t\t{c_types[type]} {name}[{name}_cnt];\n')
					output.write(f'\t\t\t\tfor(int i = 0; i < {name}_cnt; i++)\n')
					output.write(f'\t\t\t\t\t{name}[i] = _get_{type}();\n')

			# generate action
			try:
				write_indent(output, cmd.attrs["do"], 4)
			except KeyError:
				pass

			# generate output
			output.write('\t\t\t\t_reset_msg();\n')
			output.write('\t\t\t\tif(_error != OK)\n')
			output.write('\t\t\t\t\t_error = OK;\n')
			output.write('\t\t\t\telse {\n')
			output.write('\t\t\t\t\t_put_byte(OK);\n')
			for (name, type, mult) in cmd.outputs:

				# multiplicity 1
				if not mult:
					try:
						output.write(f'\t\t\t\t\t_put_{type}({cmd.attrs[name]});\n')
					except KeyError:
						output.write(f'\t\t\t\t\t_put_{type}({name});\n')

				# multiplicity n
				else:
					cnt = f"{name}_cnt"
					try:
						cnt = cmd.attrs[cnt]
					except KeyError:
						pass
					try:
						item = cmd.attrs[name]
					except KeyError:
						item = f"{name}[i]"
					output.write(f'\t\t\t\t\t_put_word({cnt});\n')
					output.write(f'\t\t\t\t\tfor(int i = 0; i < {cnt}; i++)\n')
					output.write(f'\t\t\t\t\t\t_put_{type}({item});\n')

			output.write('\t\t\t\t}\n')
			output.write('\t\t\t\t_send_msg();\n')
			output.write('\t\t\t}\n')
			output.write('\t\t\tbreak;\n')

		# switch end
		output.write("""
		default:
			raise(ERROR_BAD_COMMAND, "unknown command %d", _cmd);
		}
""")

	# generate
	try:
		text = global_attrs["c"]
	except KeyError:
		fatal("c attribute required for Python generation")
	with open(args.output, "w") as out:
		generate(out, text, {
			"includes": includes,
			"defines": defines,
			"declarations": declarations,
			"init": init,
			"receive": receive,
			#'methods': methods
		}, python=0)
