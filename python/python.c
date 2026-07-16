#define PY_SSIZE_T_CLEAN
#include <stdio.h>
#include <Python.h>
#include <csim.h>
#define FUN(n, d) 	{#n, n, METH_VARARGS, d }

#define RETURN_PTR(x)  		return PyCapsule_New(x, NULL, NULL)
#define RETURN_TPTR(t, x)	return PyCapsule_New(x, #t, NULL)
#define RETURN_NONE			Py_RETURN_NONE
#define RETURN_BOOL(x)		{ if(x) Py_RETURN_TRUE; else Py_RETURN_TRUE; }
#define RETURN_INT(x)		return Py_BuildValue("i", x)
#define RETURN_UINT(x)		return Py_BuildValue("I", x)
#define RETURN_LONG(x)		return Py_BuildValue("L", x)
#define RETURN_ULONG(x)		return Py_BuildValue("K", x)
#define RETURN_FLOAT(x)		return Py_BuildValue("f", x)
#define RETURN_DOUBLE(x)	return Py_BuildValue("d", x)
#define RETURN_STR(x)		return PyUnicode_FromString(x)

#define PTR(t, x)		((t *)PyCapsule_GetPointer(x, NULL))
#define TPTR(t, x)		((t *)PyCapsule_GetPointer(x, #t))
#define MAKE_TPTR(t, x)	PyCapsule_New(x, #t, NULL)

static PyObject *
new_board(PyObject *self, PyObject *args) {
	const char *name;
	if(!PyArg_ParseTuple(args, "s", &name))
		return NULL;
	csim_board_t *board = csim_new_board(strdup(name));
	RETURN_TPTR(csim_board_t, board);
}

static PyObject *
new_board_ext(PyObject *self, PyObject *args) {
	PyObject *oconf;
	if(!PyArg_ParseTuple(args, "O", &oconf))
		return NULL;
	if (!PyList_Check(oconf))
		return NULL;
	Py_ssize_t size = PyList_Size(oconf);
	const char *conf[size + 1];
	for(Py_ssize_t i = 0; i < size; i++) {
		PyObject *item = PyList_GetItem(oconf, i);
		if(!PyUnicode_Check(item))
			return NULL;
		conf[i] = PyUnicode_AsUTF8(item);
	}
	conf[size] = NULL;
	csim_board_t *board = csim_new_board_ext(conf);
	RETURN_TPTR(csim_board_t, board);
}

static PyObject *
delete_board(PyObject *self, PyObject *args) {
	PyObject *board;
	if(!PyArg_ParseTuple(args, "O", &board))
		return NULL;
	csim_delete_board(TPTR(csim_board_t, board));
	RETURN_NONE;
}

static PyObject *
reset_board(PyObject *self, PyObject *args) {
	PyObject *board;
	if(!PyArg_ParseTuple(args, "O", &board))
		return NULL;
	csim_reset_board(TPTR(csim_board_t, board));
	RETURN_NONE;
}

static PyObject *
run(PyObject *self, PyObject *args) {
	PyObject *board;
	uint64_t time;
	if(!PyArg_ParseTuple(args, "OK", &board, &time))
		return NULL;
	int res = csim_run(TPTR(csim_board_t, board), time);
	RETURN_INT(res);
}

static PyObject *
step(PyObject *self, PyObject *args) {
	PyObject *board;
	if(!PyArg_ParseTuple(args, "O", &board))
		return NULL;
	csim_step(TPTR(csim_board_t, board));
	RETURN_NONE;
}

static PyObject *
load_board(PyObject *self, PyObject *args) {
	const char *path;
	if(!PyArg_ParseTuple(args, "s", &path))
		return NULL;
	csim_board_t *board = csim_load_board(path);
	if(board == NULL)
		RETURN_NONE;
	else
		RETURN_TPTR(csim_board_t, board);
}

static PyObject *
get_core(PyObject *self, PyObject *args) {
	PyObject *oboard;
	if(!PyArg_ParseTuple(args, "O", &oboard))
		return NULL;
	csim_board_t *board = TPTR(csim_board_t, oboard);
	if(board->cores == NULL)
		RETURN_NONE;
	else
		RETURN_TPTR(csim_core_inst_t, board->cores);
}

static PyObject *
core_load(PyObject *self, PyObject *args) {
	PyObject *oinst;
	const char *path;
	if(!PyArg_ParseTuple(args, "Os", &oinst, &path))
		return NULL;
	int rc = csim_core_load(TPTR(csim_core_inst_t, oinst), path);
	RETURN_INT(rc);
}

static PyObject *
core_pc(PyObject *self, PyObject *args) {
	PyObject *oinst;
	if(!PyArg_ParseTuple(args, "O", &oinst))
		return NULL;
	RETURN_LONG(csim_core_pc(TPTR(csim_core_inst_t, oinst)));
}

static PyObject *
core_inst_size(PyObject *self, PyObject *args) {
	PyObject *oinst;
	if(!PyArg_ParseTuple(args, "O", &oinst))
		return NULL;
	RETURN_LONG(csim_core_inst_size(TPTR(csim_core_inst_t, oinst)));
}

static PyObject *
core_disasm(PyObject *self, PyObject *args) {
	PyObject *oinst;
	uint64_t addr;
	if(!PyArg_ParseTuple(args, "OK", &oinst, &addr))
		return NULL;
	char buf[256];
	csim_core_disasm(TPTR(csim_core_inst_t, oinst), addr, buf);
	RETURN_STR(buf);
}

static PyObject *
find_component(PyObject *self, PyObject *args) {
	const char *name;
	if(!PyArg_ParseTuple(args, "s", &name))
		return NULL;
	csim_component_t *i = csim_find_component(name);
	if(i == NULL)
		RETURN_NONE;
	else
		RETURN_TPTR(csim_component_t, i);
}

static PyObject *
component_info(PyObject *self, PyObject *args) {
	PyObject *ocomp;
	if(!PyArg_ParseTuple(args, "O", &ocomp))
		return NULL;
	csim_component_t *comp = TPTR(csim_component_t, ocomp);
	return Py_BuildValue("(siiiii)",
				comp->name,
				comp->type,
				comp->version,
				comp->reg_cnt,
				comp->port_cnt,
				comp->size);
}

static PyObject *
new_component(PyObject *self, PyObject *args) {
	PyObject *oboard;
	PyObject *ocomp;
	const char *name;
	csim_addr_t addr;
	if(!PyArg_ParseTuple(args, "OOsI", &oboard, &ocomp, &name, &addr))
		return NULL;
	csim_component_t *comp = TPTR(csim_component_t, ocomp);
	csim_board_t *board = TPTR(csim_board_t, oboard);
	RETURN_TPTR(csim_inst_t,
		csim_new_component(board, comp, name, addr));
}

static PyObject *
new_component_ext(PyObject *self, PyObject *args) {
	PyObject *oboard;
	PyObject *ocomp;
	PyObject *oconf;
	if(!PyArg_ParseTuple(args, "OOO", &oboard, &ocomp, &oconf))
		return NULL;
	csim_board_t *board = TPTR(csim_board_t, oboard);
	csim_component_t *comp = TPTR(csim_component_t, ocomp);
	if (!PyList_Check(oconf))
		return NULL;
	Py_ssize_t size = PyList_Size(oconf);
	const char *conf[size + 1];
	for(Py_ssize_t i = 0; i < size; i++) {
		PyObject *item = PyList_GetItem(oconf, i);
		if(!PyUnicode_Check(item))
			return NULL;
		conf[i] = PyUnicode_AsUTF8(item);
	}
	conf[size] = NULL;
	RETURN_TPTR(csim_inst_t,
		csim_new_component_ext(board, comp, conf));
}

static PyObject *
get_state(PyObject *self, PyObject *args) {
	PyObject *oinst;
	int size;
	if(!PyArg_ParseTuple(args, "Oi", &oinst, &size))
		return NULL;
	csim_iocomp_inst_t *i = (csim_iocomp_inst_t *)TPTR(csim_inst_t, oinst);
	uint32_t buf[size];
	((csim_iocomp_t *)(i->inst.comp))->get_state(i, buf);
	PyObject *res = PyList_New(size);
	for(int i = 0; i < size; i++)
		PyList_SetItem(res, i, PyLong_FromUnsignedLong(buf[i]));
	return res;
}

static PyObject *
set_state(PyObject *self, PyObject *args) {
	PyObject *oinst, *olist;
	if(!PyArg_ParseTuple(args, "OO", &oinst, &olist))
		return NULL;
	csim_iocomp_inst_t *i = (csim_iocomp_inst_t *)TPTR(csim_inst_t, oinst);
	int size = PyList_Size(olist);
	uint32_t state[size];
	for(int i = 0; i < size; i++)
		state[i] = PyLong_AsLong(PyList_GetItem(olist, i));
	((csim_iocomp_t *)i->inst.comp)->set_state(i, state);
	RETURN_NONE;
}

static PyObject *
find_port(PyObject *self, PyObject *args) {
	PyObject *ocomp;
	const char *name;
	if(!PyArg_ParseTuple(args, "Os", &ocomp, &name))
		return NULL;
	csim_port_t *p = csim_find_port(TPTR(csim_component_t, ocomp), name);
	if(p == NULL)
		RETURN_NONE;
	else
		RETURN_TPTR(csim_port_t, p);
}

static PyObject *
connect(PyObject *self, PyObject *args) {
	PyObject *oinst1, *oport1, *oinst2, *oport2;
	if(!PyArg_ParseTuple(args, "OOOO", &oinst1, &oport1, &oinst2, &oport2))
		return NULL;
	csim_connect(
		TPTR(csim_inst_t, oinst1),
		TPTR(csim_port_t, oport1),
		TPTR(csim_inst_t, oinst2),
		TPTR(csim_port_t, oport2)
	);
	RETURN_NONE;
}

static PyObject *
set_log_level(PyObject *self, PyObject *args) {
	PyObject *oboard;
	int level;
	if(!PyArg_ParseTuple(args, "Oi", &oboard, &level))
		return NULL;
	TPTR(csim_board_t, oboard)->level = (csim_level_t)level;
	RETURN_NONE;
}

static PyObject *
set_master_clock(PyObject *self, PyObject *args) {
	PyObject *oboard;
	csim_clock_t clock;
	if(!PyArg_ParseTuple(args, "OK", &oboard, &clock))
		return NULL;
	TPTR(csim_board_t, oboard)->clock = clock;
	RETURN_NONE;
}

static PyObject *
get_clock(PyObject *self, PyObject *args) {
	PyObject *oboard;
	if(!PyArg_ParseTuple(args, "O", &oboard))
		return NULL;
	RETURN_LONG(TPTR(csim_board_t, oboard)->clock);
}

static PyObject *
get_register(PyObject *self, PyObject *args) {
	PyObject *ocomp;
	int index;
	if(!PyArg_ParseTuple(args, "Oi", &ocomp, &index))
		return NULL;
	csim_component_t *comp = TPTR(csim_component_t, ocomp);
	RETURN_TPTR(csim_reg_t, &comp->regs[index]);
}

static PyObject *
register_info(PyObject *self, PyObject *args) {
	PyObject *oreg;
	if(!PyArg_ParseTuple(args, "O", &oreg))
		return NULL;
	csim_reg_t *reg = TPTR(csim_reg_t, oreg);
	return Py_BuildValue("(siiiiii)",
					reg->name,
					reg->offset,
					reg->size,
					reg->count,
					reg->stride,
					reg->flags,
					reg->type);
}

static PyObject *
get_register_val(PyObject *self, PyObject *args) {
	PyObject *oinst;
	PyObject *oreg;
	int index;
	if(!PyArg_ParseTuple(args, "OOi", &oinst, &oreg, &index))
		return NULL;
	csim_inst_t *inst = TPTR(csim_inst_t, oinst);
	csim_reg_t *reg = TPTR(csim_reg_t, oreg);
	RETURN_INT(reg->get(inst, index));
}

static PyObject *
set_register_val(PyObject *self, PyObject *args) {
	PyObject *oinst;
	PyObject *oreg;
	int index;
	uint32_t val;
	if(!PyArg_ParseTuple(args, "OOii", &oinst, &oreg, &index, &val))
		return NULL;
	csim_inst_t *inst = TPTR(csim_inst_t, oinst);
	csim_reg_t *reg = TPTR(csim_reg_t, oreg);
	reg->set(inst, index, val);
	RETURN_NONE;
}

static PyObject *
version(PyObject *self, PyObject *args) {
	RETURN_INT(1);
}

static PyObject *
core_inst(PyObject *self, PyObject *args) {
	PyObject *oinst;
	if(!PyArg_ParseTuple(args, "O", &oinst))
		return NULL;
	csim_core_inst_t *inst = TPTR(csim_core_inst_t, oinst);
	RETURN_TPTR(csim_inst_t, &inst->inst);
}

static PyObject *
byte_at(PyObject *self, PyObject *args) {
	PyObject *oboard;
	csim_addr_t addr;
	if(!PyArg_ParseTuple(args, "OI", &oboard, &addr))
		return NULL;
	csim_board_t *board = TPTR(csim_board_t, oboard);
	RETURN_UINT(csim_byte_at(board, addr));
}

static PyObject *
half_at(PyObject *self, PyObject *args) {
	PyObject *oboard;
	csim_addr_t addr;
	if(!PyArg_ParseTuple(args, "OI", &oboard, &addr))
		return NULL;
	csim_board_t *board = TPTR(csim_board_t, oboard);
	RETURN_UINT(csim_half_at(board, addr));
}

static PyObject *
word_at(PyObject *self, PyObject *args) {
	PyObject *oboard;
	csim_addr_t addr;
	if(!PyArg_ParseTuple(args, "OI", &oboard, &addr))
		return NULL;
	csim_board_t *board = TPTR(csim_board_t, oboard);
	RETURN_UINT(csim_word_at(board, addr));
}

static PyObject *
long_at(PyObject *self, PyObject *args) {
	PyObject *oboard;
	csim_addr_t addr;
	if(!PyArg_ParseTuple(args, "OI", &oboard, &addr))
		return NULL;
	csim_board_t *board = TPTR(csim_board_t, oboard);
	RETURN_ULONG(csim_long_at(board, addr));
}

static PyObject *
get_date(PyObject *self, PyObject *args) {
	PyObject *oboard;
	if(!PyArg_ParseTuple(args, "O", &oboard))
		return NULL;
	csim_board_t *board = TPTR(csim_board_t, oboard);
	RETURN_ULONG(board->date);
}

static PyObject *
inst_id(PyObject *self, PyObject *args) {
	PyObject *oinst;
	if(!PyArg_ParseTuple(args, "O", &oinst))
		return NULL;
	RETURN_ULONG(TPTR(csim_inst_t, oinst)->id);
}

static PyObject *
register_make_name(PyObject *self, PyObject *args) {
	PyObject *oinst;
	PyObject *oreg;
	int index;
	if(!PyArg_ParseTuple(args, "OOi", &oinst, &oreg, &index))
		return NULL;
	csim_reg_t *reg = TPTR(csim_reg_t, oreg);
	csim_inst_t *inst = TPTR(csim_inst_t, oinst);
	char buf[256];
	reg->make_name(inst, index, buf, 256);
	RETURN_STR(buf);
}

static PyObject *
flush_iostates(PyObject *self, PyObject *args) {
	PyObject *oboard;
	if(!PyArg_ParseTuple(args, "O", &oboard))
		return NULL;
	csim_board_t *board = TPTR(csim_board_t, oboard);
	int count = board->iostates_count;
	csim_ioinfo_t infos[count];
	csim_flush_iostates(board, infos);
	PyObject *list = PyList_New(count);
	for(int i = 0; i < count; i++) {
		PyObject *tuple = PyTuple_New(3);
		PyTuple_SetItem(tuple, 0, PyLong_FromLong(infos[i].id));
		PyTuple_SetItem(tuple, 1, PyLong_FromLong(infos[i].ress));
		PyTuple_SetItem(tuple, 2, PyLong_FromLong(infos[i].state));
		PyList_SetItem(list, i, tuple);
	}
	return list;
}

static PyObject *
do_input(PyObject *self, PyObject *args) {
	PyObject *oboard;
	csim_ioinfo_t info;
	if(!PyArg_ParseTuple(args, "Oiii", &oboard, &info.id, &info.ress, &info.state))
		return NULL;
	csim_do_input(TPTR(csim_board_t, oboard), &info);
	RETURN_NONE;
}

static PyObject *
set_break(PyObject *self, PyObject *args) {
	PyObject *ocore;
	csim_addr_t addr;
	if(!PyArg_ParseTuple(args, "OI", &ocore, &addr))
		return NULL;
	//printf("DEBUG:PyC: set_break %08x\n", addr);
	csim_core_inst_t *core = TPTR(csim_core_inst_t, ocore);
	((csim_core_t *)core->inst.comp)->set_break(core, addr);
	RETURN_NONE;
}

static PyObject *
clear_break(PyObject *self, PyObject *args) {
	PyObject *ocore;
	csim_addr_t addr;
	if(!PyArg_ParseTuple(args, "OI", &ocore, &addr))
		return NULL;
	csim_core_inst_t *core = TPTR(csim_core_inst_t, ocore);
	((csim_core_t *)core->inst.comp)->clear_break(core, addr);
	RETURN_NONE;
}

static PyObject *
board_confs(PyObject *self, PyObject *args) {
	PyObject *oboard;
	if(!PyArg_ParseTuple(args, "O", &oboard))
		return NULL;
	csim_board_t *board = TPTR(csim_board_t, oboard);
	int cnt = 0;
	while(board->confs[cnt])
		cnt++;
	PyObject *list = PyList_New(cnt);
	for(int i = 0; i < cnt; i++)
		PyList_SET_ITEM(list, i, PyUnicode_FromString(board->confs[i]));
	return list;
}

static PyObject *
inst_confs(PyObject *self, PyObject *args) {
	PyObject *oinst;
	if(!PyArg_ParseTuple(args, "O", &oinst))
		return NULL;
	csim_inst_t *inst = TPTR(csim_inst_t, oinst);
	int cnt = 0;
	while(inst->confs[cnt])
		cnt++;
	PyObject *list = PyList_New(cnt);
	for(int i = 0; i < cnt; i++)
		PyList_SET_ITEM(list, i, PyUnicode_FromString(inst->confs[i]));
	return list;
}

static PyObject *
get_insts(PyObject *self, PyObject *args) {
	PyObject *oboard;
	if(!PyArg_ParseTuple(args, "O", &oboard))
		return NULL;
	csim_board_t *board = TPTR(csim_board_t, oboard);
	PyObject *list = PyList_New(board->inst_cnt);
	for(unsigned i = 0; i < board->inst_cnt; i++)
		PyList_SET_ITEM(list, i, MAKE_TPTR(csim_inst_t, board->insts[i]));
	return list;
}

static PyObject *
get_comp(PyObject *self, PyObject *args) {
	PyObject *oinst;
	if(!PyArg_ParseTuple(args, "O", &oinst))
		return NULL;
	csim_inst_t *inst = TPTR(csim_inst_t, oinst);
	RETURN_TPTR(csim_component_t, inst->comp);
}

static PyObject *
inst_info(PyObject *self, PyObject *args) {
	PyObject *oinst;
	if(!PyArg_ParseTuple(args, "O", &oinst))
		return NULL;
	csim_inst_t *inst = TPTR(csim_inst_t, oinst);
	return Py_BuildValue("(isiii)",
		inst->base,
		inst->name,
		inst->number,
		inst->flags,
		inst->id);
}

static PyObject *
board_name(PyObject *self, PyObject *args) {
	PyObject *oboard;
	if(!PyArg_ParseTuple(args, "O", &oboard))
		return NULL;
	csim_board_t *board = TPTR(csim_board_t, oboard);
	RETURN_STR(board->name);
}

static PyMethodDef csim_methods[] = {
	FUN(new_board, "(name, memory) Create a new board."
		"memory may be None. Return the board."),
	FUN(delete_board, "(board) Delete the given board"),
	FUN(reset_board, "(board): reset the board."),
	FUN(run, "(board, time) Run the board during time cycles."),
	FUN(step, "(board) Execute the current instruction."),
	FUN(load_board, "(path) Load the given executable and build/return the board"),
	FUN(get_core, "(board) Get the execution core of the board (may return None if there is no core)."),
	FUN(core_load, "(core instance, path) Load the executable from the path into the board containing the core."
			"Returns 0 for success or an error code."),
	FUN(core_pc, "(core instance) Get the current PC address of the given core instance."),
	FUN(core_disasm, "(core instance, address) Return the disassembly of the instruction at the given address."),
	FUN(find_component, "(name) Look for a component by its name. Return component or None."),
	FUN(component_info, "(component) Get information about the component as the tuple (name, type, version, register count, port count, instance size,)."),
	FUN(new_component, "(board, component, name, base address) Build a new instance of the component."),
	FUN(get_state, "(instance, size) Get state from an IO component. The result is a list of size integers."),
	FUN(set_state, "(instance, state) Set the state of an IO component instance. state is a list of integers."),
	FUN(find_port, "(component, name) Look for a port with a named in the component. Return found port or None."),
	FUN(connect, "(instance 1, port 1, instance 2, port 2) Connect the port of instance 1 with the port of instance 2."),
	FUN(set_log_level, "(board, level) Set the log level (level is an integer as CSIM_NOLOG=0, CSIM_DEBUG=1, etc)."),
	FUN(set_master_clock, "(board, clock) Set the master clock of the board."),
	FUN(get_register, "(component, index) Get the register for the given index."),
	FUN(register_info, "(register) Return register information (name, offset, size, count, stride, flags, type)."),
	FUN(get_register_val, "(instance, register, index) Get the value of a register."),
	FUN(set_register_val, "(instance, register, index, value) Set the value of a register."),
	FUN(core_inst, "(core instance) Get the component instance of the passed core instance."),
	FUN(core_inst_size, "(core instance) Get the size of the current instruction in the core."),
	FUN(version, "Get version."),
	FUN(byte_at, "(board, address) Get the byte in the board at the passed address."),
	FUN(half_at, "(board, address) Get the half-word in the board at the passed address."),
	FUN(word_at, "(board, address) Get the word in the board at the passed address."),
	FUN(long_at, "(board, address) Get the long word in the board at the passed address."),
	FUN(get_date, "(board) Get the date of the board."),
	FUN(register_make_name, "(component, register, index) Build the name for register at index."),
	FUN(new_board_ext, "(configuration: string list) Build a board with  configuration."),
	FUN(new_component_ext, "(board, component, configuration: string list) build a new component instance."),
	FUN(get_clock, "(board) get the master clock of the board."),
	FUN(flush_iostates, "(board): list of (number: int, id: int, state: int) Get the IO states for updating."),
	FUN(inst_id, "(instance) Get the identifier of the instance."),
	FUN(do_input, "(board, id, ressource, state) Perform an input as a state change."),
	FUN(set_break, "(core, address) Set a breakpoint."),
	FUN(clear_break, "(core, address) Clear a breakpoint."),
	FUN(board_confs, "(board) Get configurations for the board: an array made of a sequence of ID, value."),
	FUN(inst_confs, "(board) Get configurations for an instance. an array made of a sequence of ID, value."),
	FUN(get_insts, "(board) Get the list of components in the board."),
	FUN(get_comp, "(instance) Get the component of the instance."),
	FUN(inst_info, "(instance) Get information about instance (base address, name, number, flags, id)"),
	FUN(board_name, "(board) Get the name of the board."),
	{NULL, NULL, 0, NULL}
};

static struct PyModuleDef csim_module = {
    PyModuleDef_HEAD_INIT,
    "libcsim",
    "Compmonent Simulation Library",
    -1,
    csim_methods
};

PyMODINIT_FUNC
PyInit_libcsim(void) {
    return PyModule_Create(&csim_module);
}
