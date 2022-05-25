bin_PROGRAMS=evbtagger

evbtagger_SOURCES=evbtagger.cpp evbtagger.h

BUILT_SOURCES=evbtagger.cpp evbtagger.h

evbtagger.cpp: evbtagger.h

evbtagger.h: evbtagger.ggo
	$(GENGETOPT) <@srcdir@/evbtagger.ggo -c cpp -F evbtagger
