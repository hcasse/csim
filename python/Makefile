
include ../config.mk

MODULE = libcsim
PYTHON = $(shell which python3)
PYTHON_CONFIG = python3-config
LIBEXT=$(shell $(PYTHON_CONFIG) --extension-suffix)
CFLAGS = \
	-fPIC \
	$(shell $(PYTHON_CONFIG) --cflags) \
	-I ../lib
LDFLAGS = \
	$(shell $(PYTHON_CONFIG) --ldflags) \
	-L../lib -lcsim
ifdef ARMV5T_PATH
LDFLAGS += -L../$(ARMV5T_PATH)/src -larm
endif

export PYTHONPATH=$(WITH_ORCHID):$(PWD)

NUM = 1

all: $(MODULE)$(LIBEXT)

$(MODULE)$(LIBEXT): python.c ../lib/csim.c
	gcc $(CFLAGS) -shared -o $@ $< $(LDFLAGS)

$(MODULE)$(LIBEXT): ../*.c

clean:
	-rm -rf $(MODULE)$(LIBEXT)

export PYTHONPATH=$(PWD):$(PWD)/../../Orchid
export CSIMPATH=
ifdef WITH_ALTERA
CSIMPATH+=:$(PWD)/../altera
endif
export LD_LIBRARY_PATH=$(PWD):$(PWD)/../lib

env:
	@echo "PYTHONPATH=$$PYTHONPATH"
	@echo "CSIMPATH=$$CSIMPATH"
	@echo "LD_LIBRARY_PATH=$$LD_LIBRARY_PATH"

run: env
	$(PYTHON) run.py ../samples/$(TEST).yaml ../samples/$(TEST).elf

debug: env
	$(PYTHON) run.py ../samples/$(TEST).yaml ../samples/$(TEST).elf --debug

show: env
	$(PYTHON) run.py $(BOARD) $(EXEC) --debug

test-server: env
	$(PYTHON) test-server.py


# automatic documentation
autodoc:
	gnome-terminal -- pydoc3 -b
