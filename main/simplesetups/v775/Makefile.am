DOCS    = @srcdir@/v775setup.xml @srcdir@/v775bookinfo.xml
FIGURES = @srcdir@/v775ediagram.jpeg @srcdir@/v775ediagram.eps
FIGSRC  = @srcdir@/v775ediagram.dia


CHAPTER2BOOK=@PYTHON@ @top_srcdir@/docbuild/chapter2book.py

BOOK = @builddir@/v775setup_autobook.pdf

install-exec-local:
	$(mkinstalldirs) @prefix@/share/html
	$(mkinstalldirs) @prefix@/share/pdfs
	$(INSTALL_DATA) $(FIGURES) @prefix@/share/html
	$(CHAPTER2BOOK) -s @srcdir@/v775setup.xml -b @srcdir@/v775bookinfo.xml -o @builddir@ 
	$(INSTALL_DATA) $(BOOK) @prefix@/share/pdfs

clean-local:
	rm -f $(BOOK)

EXTRA_DIST=$(DOCS) $(FIGURES) $(FIGSRC)
