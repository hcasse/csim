GLISS = ../gliss/gep/gliss-csim
GLISS_FLAGS = \
	-h ../templates/comp.h \
	-c ../templates/comp.c
CFLAGS = -I.. -g3 -fPIC
LDFLAGS =

NMPS = $(wildcard *.nmp)
HEADERS = $(NMPS:.nmp=.h)
SOURCES = $(NMPS:.nmp=.c)
OBJECTS = $(NMPS:.nmp=.o)

all: libaltera.so

clean:
	-rm -f $(HEADERS) $(SOURCES) $(OBJECTS) altera.o libaltera.so

