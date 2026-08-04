GLISS = ../gliss/gep/gliss-csim
GLISS_FLAGS = \
	-h ../templates/comp.h \
	-c ../templates/comp.c
CFLAGS = -I../lib -g3 -fPIC
LDFLAGS = -L../lib -lcsim

NMPS = $(wildcard *.nmp)
HEADERS = $(NMPS:.nmp=.h)
SOURCES = $(NMPS:.nmp=.c)
OBJECTS = $(NMPS:.nmp=.o) altera.o

all: libaltera.so

libaltera.so: $(OBJECTS) ../libcsim.so
	gcc $(CFLAGS) -shared -o $@ $(OBJECTS) $(LDFLAGS)

clean:
	-rm -f $(HEADERS) $(SOURCES) $(OBJECTS) altera.o libaltera.so

%.c: %.nmp
	$(GLISS) $(GLISS_FLAGS) $<

.PRECIOUS: $(SOURCES)
