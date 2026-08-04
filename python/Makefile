
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
	-L.. -lcsim
ifdef ARMV5T_PATH
LDFLAGS += -L../$(ARMV5T_PATH)/src -larm
endif

export PYTHONPATH=$(WITH_ORCHID):$(PWD)

NUM = 1

all: $(MODULE)$(LIBEXT)

$(MODULE)$(LIBEXT): python.c ../lib/csim.c
	gcc $(CFLAGS) -shared -o $@ $< $(LDFLAGS)
	#-Wl,--whole-archive ../libcsim.a

$(MODULE)$(LIBEXT): ../*.c

clean:
	-rm -rf $(MODULE)$(LIBEXT)

export PYTHONPATH=$(PWD):$(PWD)/../../Orchid
export CSIMPATH=$(PWD)/../altera
export LD_LIBRARY_PATH=$(PWD):$(PWD)/../lib

run:
	@echo "PYTHONPATH=$$PYTHONPATH"
	@echo "CSIMPATH=$$CSIMPATH"
	@echo "LD_LIBRARY_PATH=$$LD_LIBRARY_PATH"
	$(PYTHON) run.py ../samples/$(SAMPLE).yaml ../samples/$(SAMPLE).elf

debug:
	@echo "PYTHONPATH=$$PYTHONPATH"
	@echo "CSIMPATH=$$CSIMPATH"
	@echo "LD_LIBRARY_PATH=$$LD_LIBRARY_PATH"
	$(PYTHON) run.py ../samples/$(SAMPLE).yaml ../samples/$(SAMPLE).elf --debug

autodoc:
	gnome-terminal -- pydoc3 -b
