EXAMPLES=ReadNSCLDAQFiles

install-data-local:
	$(mkinstalldirs) @datarootdir@
	$(mkinstalldirs) @datarootdir@/examples
	for dir  in $(EXAMPLES) ; do                           \
		$(mkinstalldirs) @datarootdir@/examples/$$dir; \
		for f in $$dir/*; do                           \
			install $$f @datarootdir@/examples/$$dir;                     \
		done                                           \
	done

EXTRA_DIST = $(EXAMPLES)
