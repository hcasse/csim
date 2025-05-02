ARCH=arm-linux-gnueabihf-
CC=$(ARCH)gcc
CFLAGS=-g
LDFLAGS=-nostartfiles -static
SAMPLES= $(wildcard *.s)

all: $(SAMPLES:.s=.elf)

clean: 
	-rm -f $(SAMPLES:.s=.elf)

%.elf: %.s
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)
