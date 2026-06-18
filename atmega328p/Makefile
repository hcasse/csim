GLISS = ../gliss/gep/gliss-csim
CFLAGS = -I.. -g

NMPS = oscillator.nmp


all: libatmega328p.a

clean:
	-rm $(NMPS:.nmp=.o) libatmega328p.a
	cd samples; make clean
	
libatmega328p.a: $(NMPS:.nmp=.o)
	ar rcs $@ $^

%.c: %.nmp
	$(GLISS) $<
	mv $@ $(@:.c=.h) ../ 
	cd samples; make

# Commandes pour tester la génération des composants de nml vers c et h via GLISS.
# Les tests de génération doivent se faire directement dans le dossier atmega328p.
# Pour nettoyer à la fin du développement, utiliser la commande dev-clean.

dev:
	@if [ -z "$(f)" ]; then \
		echo "Error: please specify a .nmp file with f=filename.nmp"; \
		exit 1; \
	fi
	$(GLISS) $(f)

dev-clean:
	-rm *.c *.h
