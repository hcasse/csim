LIB = stm32

NMPS = \
	GPIO.nmp

GLISS = ../gliss/gep/gliss-csim
CFLAGS = -I.. -g

all: lib$(LIB).a

clean:
	-rm $(NMPS:.nmp=.o) lib$(LIB).a

distclean: clean
	-rm $(NMPS:.nmp=.h) $(NMPS:.nmp=.c)

libstm32.a: $(NMPS:.nmp=.o)
	ar rcs $@ $^

%.c: %.nmp
	$(GLISS) $<

.PRECIOUS: GPIO.c
