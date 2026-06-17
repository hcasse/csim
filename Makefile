-include config.mk

# Configuration
YAML=$(PWD)/easy-yaml

HEADERS=csim.h
COMPONENTS= seven_seg_controller.c seven_seg_display.c led.c button.c leds10.c leds10.c timer.c
SOURCES=csim.c yaml.c  csim-rt.o arm_core.c loader.c $(COMPONENTS)

CFLAGS=-g3 -Wall -fPIC -I. -DCOMPAT
LDFLAGS=-L. -lcsim

# additional goals
ALL =
CLEAN =
DISTCLEAN =
ifdef WITH_STM32
ALL += stm32-all
CLEAN += stm32-clean
DISTCLEAN += stm32-distclean
endif

# Orchid options
ifdef WITH_ORCHID
ifdef (WITH_ORCHID,)
ORCHID_PATH=Orchid
ORCHID_SETUP=git-orchid
else
ORCHID_PATH=$(WITH_ORCHID)
endif
endif

# ARMV5T option
ifdef WITH_ARMV5T
ifeq (WITH_ARMV5T,)
ARMV5T_PATH=armv5t
ARMV5T_SETUP=armv5t-setup
else
ARMV5T_PATH=$(WITH_ARMV5T)
endif
CFLAGS += -DNO_MEM -I$(ARMV5T_PATH)/include
LDFLAGS += -L$(ARMV5T_PATH)/src -larm
else
SOURCES += mem.c
endif


# Python option
ifdef WITH_PYTHON
ALL += with-python
endif


# useful definitions
OBJECTS=$(SOURCES:.c=.o)


# rules
all: gliss-all libcsim.a test-csim csim-run $(ALL)

clean: gliss-clean $(CLEAN)
	-rm -rf $(OBJECTS) test-csim.o test-csim

distclean: clean $(DISTCLEAN)
	-rm -rf test-csim test2 libcsim.a

test-csim: test-csim.o libcsim.a
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

csim-run: csim-run.o libcsim.a
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

libcsim.a: $(OBJECTS)
	ar rcs $@ $(OBJECTS)

csim.o: csim.h mem.h
mem.o: mem.h
test-csim.o: csim.h
yaml.o: yaml.h
test2.o: csim.h mem.h yaml.h led.h button.h
csim-rt.o: csim-rt.h
loader.o: csim.h yaml.h
%.o: $(COMPONENTS)

FILES = \
	csim/README.md \
	csim/Makefile \
	csim/samples/Makefile \
	csim/samples/sample1.s \
	csim/samples/sample1.elf

dist: distclean
	cd ..; tar cvfz csim.tgz $(FILES) csim/*.c csim/*.h

gliss-all:
	cd gliss; make all

gliss-clean:
	cd gliss; make clean


# STM32 rules
stm32-all:
	cd stm32; make all

stm32-clean:
	cd stm32; make clean


# python rules
with-python:
	cd python; make

# setup
GLISS_GIT = https://git.renater.fr/anonscm/git/gliss2/gliss2.git
ARMV5T_GIT = https://git.renater.fr/anonscm/git/gliss2/armv5t.git
ORCHID_GIT = https://github.com/hcasse/Orchid.git

setup: $(ARMV5T_SETUP) $(ORCHID_SETUP)

git-gliss:
	@if [ -e gliss2 ]; then \
		echo "Updating gliss"; \
		cd gliss2; git pull; \
	else \
		echo "Downloading gliss"; \
		git clone $(GLISS_GIT); \
	fi
	@cd gliss2; make

git-armv5t: git-gliss
	@if [ -e armv5t ]; then \
		echo "Updating ArmV5T"; \
		cd armv5t; git pull; \
	else \
		echo "Downloading ArmV5T"; \
		git clone $(ARMV5T_GIT); \
		cd armv5t; \
		make config.mk; \
		echo "WITH_IO = 1" >> config.mk; \
	fi
	@cd armv5t; make

git-orchid:
	@if [ -e Orchid ]; then \
		echo "Updating Orchid"; \
		cd Orchid; git pull; \
	else \
		echo "Downloading Orchid"; \
		git clone $(ORCHID_GIT); \
	fi

config: config-force setup


config.mk: config.in
	cp config.in config.mk
ifdef WITH_ARMV5T
	echo "WITH_ARMV5T=$(WITH_ARMV5T)" >> config.mk
endif
ifdef WITH_ORCHID
	echo "WITH_ORCHID=$(WITH_ORCHID)" >> config.mk
endif



