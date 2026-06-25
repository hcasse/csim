#ARCH=arm-linux-gnueabihf-
ARCH=arm-linux-gnueabi-
CC=$(ARCH)gcc
CFLAGS=-g
LDFLAGS=-nostartfiles -static
OBJDUMP=$(ARCH)objdump
SAMPLES= $(wildcard *.s)

all: $(SAMPLES:.s=.elf)

clean:
	-rm -f $(SAMPLES:.s=.elf)

%.elf: %.s
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

%.dis: %.elf
	$(OBJDUMP) -d $< > $@
