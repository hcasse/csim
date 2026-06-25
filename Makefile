-include config.mk

# Configuration
YAML=$(PWD)/easy-yaml

HEADERS=\
	csim.h
SOURCES=\
	csim.c \
	yaml.c \
	csim-rt.o \
	arm_core.c \
	led.c \
	button.c \
	loader.c

CFLAGS=-g3 -Wall -fPIC -I. -DCOMPAT
LDFLAGS=-L. -lcsim

# additional goals
ALL =
CLEAN =
DISTCLEAN =

# STM32 option
ifdef WITH_STM32
ALL += stm32-all
CLEAN += stm32-clean
DISTCLEAN += stm32-distclean
endif

# AVR option
ifdef WITH_ATMEGA328P
ALL += atmega328p-all
CLEAN += atmega328p-clean
DISTCLEAN += atmega328p-distclean
endif

# ARMV5T option
ifdef ARMV5T_PATH
CFLAGS += -DNO_MEM -I$(ARMV5T_PATH)/include
LDFLAGS += -L$(ARMV5T_PATH)/src -larm
endif

# Python option
ifdef WITH_PYTHON
ALL += python-all
CLEAN += python-clean
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


# ATMEGA328P rules
atmega328p-all:
	cd atmega328p; make all

atmega328p-clean:
	cd atmega328p; make clean

# python rules
python-all:
	cd python; make

python-clean:
	cd python; make clean

# setup
GLISS_GIT = https://git.renater.fr/anonscm/git/gliss2/gliss2.git
ARMV5T_GIT = https://git.renater.fr/anonscm/git/gliss2/armv5t.git
ORCHID_GIT = https://github.com/hcasse/Orchid.git

setup: git-armv5t git-orchid

git-gliss:
ifeq ($(ARMV5T_PATH),armv5t)
	@if [ -e gliss2 ]; then \
		echo "GLISS2 already setup!"; \
	else \
		echo "Downloading gliss"; \
		git clone -b csim $(GLISS_GIT); \
	fi
	@cd gliss2; make
endif

git-armv5t: git-gliss
ifeq ($(ARMV5T_PATH),armv5t)
	@if [ -e armv5t ]; then \
		echo "ArmV5T already setup!"; \
	else \
		echo "Downloading ArmV5T"; \
		git clone $(ARMV5T_GIT); \
		cd armv5t; \
		make config.mk; \
		echo "WITH_IO = 1" >> config.mk; \
	fi
	@cd armv5t; make
endif

git-orchid:
ifeq ($(ORCHID_PATH),Orchid)
	@if [ -e Orchid ]; then \
		echo "Orchid already setup!"; \
	else \
		echo "Downloading Orchid"; \
		git clone $(ORCHID_GIT); \
	fi
endif


ifdef WITH_ARMV5T
ARMV5T_PATH=armv5t
endif

ifdef WITH_ORCHID
ORCHID_PATH=Orchid
endif

config: config.mk

config.mk:
	cp config.in config.mk

ifdef ARMV5T_PATH
	echo "ARMV5T_PATH=$(ARMV5T_PATH)" >> config.mk
endif
ifdef WITH_ORCHID
	echo "ORCHID_PATH=$(ORCHID_PATH)" >> config.mk
endif



