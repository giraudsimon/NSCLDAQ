SUBDIRS=dpp-pha dpp-psd caendump


install-exec-hook:
	$(mkinstalldirs) @prefix@/share/caendpp
	$(DOCBOOK2HTML) -o @prefix@/share/caendpp @srcdir@/caendigitizers.xml


EXTRA_DIST=caendigitizers.xml
