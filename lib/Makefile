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

clean:
	-rm -rf $(ALL_OBJECTS) test-csim.o test-csim

distclean: clean
	-rm -rf libcsim.so $(APPS)

test-csim: test-csim.o libcsim.so
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

csim-run: csim-run.o libcsim.so
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

csim-server: server.o libcsim.so
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

libcsim.so: $(LIB_OBJECTS)
	gcc -shared -o $@ $(LIB_OBJECTS)

server.c: server.desc gen.py
	./gen.py -c $< $@

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
export CSIMPATH=$(PWD)
ifdef WITH_ALTERA
CSIMPATH += :$(PWD)/../altera
endif
export LD_LIBRARY_PATH=$(PWD)

run:
	@echo "CSIMPATH=$$CSIMPATH"
	@echo "LD_LIBRARY_PATH=$$LD_LIBRARY_PATH"
	./csim-run ../samples/$(TEST).yaml ../samples/$(TEST).elf

debug:
	gdb --args ./csim-run ../samples/$(TEST).yaml ../samples/$(TEST).elf

