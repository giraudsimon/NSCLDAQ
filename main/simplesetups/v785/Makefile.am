DOCS    = @srcdir@/v785setup.xml @srcdir@/v785bookinfo.xml
FIGURES = @srcdir@/figures/ss785_amplified_signal_with_gate.png \
					@srcdir@/figures/ss785_amplified_signal_with_logic_pulse.png \
					@srcdir@/figures/ss785_amplifier_output.png \
					@srcdir@/figures/ss785_electronics_block_diagram_new.png \
					@srcdir@/figures/ss785_pulser_output.png


CHAPTER2BOOK=@PYTHON@ @top_srcdir@/docbuild/chapter2book.py

BOOK = @builddir@/v785setup_autobook.pdf

install-exec-local:
	$(mkinstalldirs) @prefix@/share/html
	$(mkinstalldirs) @prefix@/share/html/figures
	$(mkinstalldirs) @prefix@/share/pdfs
	$(INSTALL_DATA) $(FIGURES) @prefix@/share/html/figures
	$(CHAPTER2BOOK) -s @srcdir@/v785setup.xml -b @srcdir@/v785bookinfo.xml -o @builddir@
	$(INSTALL_DATA) $(BOOK) @prefix@/share/pdfs

clean-local:
	rm -f $(BOOK)

EXTRA_DIST=$(DOCS) $(FIGURES)
