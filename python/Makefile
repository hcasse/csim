
include ../config.mk

MODULE = libcsim
PYTHON = $(shell which python3)
PYTHON_CONFIG = python3-config
LIBEXT=$(shell $(PYTHON_CONFIG) --extension-suffix)
CFLAGS = \
	-fPIC \
	$(shell $(PYTHON_CONFIG) --cflags) \
	-I ..
LDFLAGS = \
	$(shell $(PYTHON_CONFIG) --ldflags) \
	-L .. -lcsim
ifdef ARMV5T_PATH
LDFLAGS += -L../$(ARMV5T_PATH)/src -larm
endif

export PYTHONPATH=$(WITH_ORCHID):$(PWD)

NUM = 1

all: $(MODULE)$(LIBEXT)

$(MODULE)$(LIBEXT): python.c
	gcc $(CFLAGS) -shared -o $@ $< $(LDFLAGS)

$(MODULE)$(LIBEXT): ../libcsim.a

clean:
	-rm -rf $(MODULE)$(LIBEXT)

run:
	$(PYTHON) run.py ../samples/sample$(NUM).yaml ../samples/sample$(NUM).elf

autodoc:
	gnome-terminal -- pydoc3 -b
