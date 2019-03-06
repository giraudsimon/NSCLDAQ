bin_PROGRAM = ringFragmentSource

BUILT_SOURCES = rfcmdline.c rfcmdline.h



rfcmdline.c: rfcmdline.h


rfcmdline.h: options.ggo
	$(GENGETOPT) --output-dir=@builddir@ \
		--input=@builddir@/options.ggo -F rfcmdline
