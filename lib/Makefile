-include ../config.mk

# Configuration
HEADERS=\
	csim.h
SOURCES=\
	csim.c \
	yaml.c \
	csim-rt.o \
	arm_core.c \
	led.c \
	button.c \
	loader.c \
	sseg.c

CFLAGS=-g3 -Wall -fPIC -I. -DCOMPAT
LDFLAGS=-L. -lcsim

PLUGINS =
export CSIMPATH=:.$(patsubst %,:../%,$(PLUGINS))
export PYTHONPATH=$(PWD)/../python:../../Orchid

# ARMV5T option
ifdef ARMV5T_PATH
CFLAGS += -I../$(ARMV5T_PATH)/include
LDFLAGS += -L../$(ARMV5T_PATH)/src -larm
endif

# useful definitions
APPS=test-csim csim-run csim-server
LIB_OBJECTS=$(SOURCES:.c=.o)
ALL_OBJECTS=$(LIB_OBJECTS) $(patsubst %,%.o,$(APPS))


# rules
all: libcsim.so $(APPS)

clean: plugins-clean
	-rm -rf $(ALL_OBJECTS) test-csim.o test-csim

distclean: clean $(DISTCLEAN)
	-rm -rf libcsim.so $(APPS)

test-csim: test-csim.o libcsim.so
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

csim-run: csim-run.o libcsim.so
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

csim-server: server.o libcsim.so
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

libcsim.so: $(LIB_OBJECTS)
	gcc -shared -o $@ $(LIB_OBJECTS)

# source dependencies
csim.o: csim.h
test-csim.o: csim.h
yaml.o: yaml.h
test2.o: csim.h yaml.h led.h button.h
csim-rt.o: csim-rt.h
loader.o: csim.h yaml.h
arm_core.o: csim.h
led.o: csim.h
button.o: csim.h
csim-run.o: csim.h
server.o: csim.h
sseg.o: sseg.h csim.h


# testing
run:
	@echo "CSIMPATH=$$CSIMPATH"
	@echo "PYTHONPATH=$$PYTHONPATH"
	./python/run.py ../samples/$(TEST).yaml ../samples/$(TEST).elf



