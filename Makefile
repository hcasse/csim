-include config.mk

# Configuration
PLUGINS =
SUBDIRS = lib gliss

# Altera option
ifdef WITH_ALTERA
PLUGINS += altera
endif

SUBDIRS += $(PLUGINS)

# STM32 option
#ifdef WITH_STM32
#ALL += stm32-all
#CLEAN += stm32-clean
#DISTCLEAN += stm32-distclean
#endif

# AVR option
#ifdef WITH_ATMEGA328P
#ALL += atmega328p-all
#CLEAN += atmega328p-clean
#DISTCLEAN += atmega328p-distclean
#endif

# Python option
ifdef WITH_PYTHON
SUBDIRS += python
endif

# rules
all:
	echo $(SUBDIRS)
	for d in $(SUBDIRS); do cd $$d; make || exit 1; cd ..; done

clean:
	for d in $(SUBDIRS); do cd $$d; make clean; cd ..; done

distclean:
	for d in $(SUBDIRS); do cd $$d; make distclean; cd ..; done

# testing
run:
	@echo "CSIMPATH=$$CSIMPATH"
	@echo "PYTHONPATH=$$PYTHONPATH"
	./python/run.py samples/$(TEST).yaml samples/$(TEST).elf


# STM32 rules
#stm32-all:
#	cd stm32; make all

#stm32-clean:
#	cd stm32; make clean


# Altera


# ATMEGA328P rules
#atmega328p-all:
#	cd atmega328p; make all

#atmega328p-clean:
#	cd atmega328p; make clean


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



