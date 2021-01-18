SUBDIRS=dpp-pha dpp-psd caendump tweaker


if BUILD_DOCS
install-exec-hook:
	$(mkinstalldirs) @prefix@/share/caendpp
	$(DOCBOOK2HTML) -o @prefix@/share/caendpp @srcdir@/caendigitizers.xml
endif

EXTRA_DIST=caendigitizers.xml
