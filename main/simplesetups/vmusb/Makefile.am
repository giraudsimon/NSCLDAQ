SUBDIRS = src

DOCS    = @srcdir@/vmusbsetup.xml @srcdir@/vmusb_bookinfo.xml
FIGURES =  @srcdir@/figures/sis3820blockdiagram.dia  \
					 @srcdir@/figures/sis3820blockdiagram.jpg \
					 @srcdir@/figures/sis3820blockdiagram.eps \
					 @srcdir@/figures/sis3820blockdiagram.png \
					 @srcdir@/figures/v785v775sis3820setup.png \
					 @srcdir@/figures/v785v775sis3820setup.eps \
					 @srcdir@/figures/v785v775sis3820setup.dia \
					 @srcdir@/figures/treegui_1d_0.png \
					 @srcdir@/figures/treegui_1d_1.png  \
					 @srcdir@/figures/treegui_1d_2.png \
					 @srcdir@/figures/treegui_connect.png \
					 @srcdir@/figures/xamine_0.png \
					 @srcdir@/figures/xamine_1.png 



CHAPTER2BOOK=@PYTHON@ @top_srcdir@/docbuild/chapter2book.py

BOOK = @builddir@/vmusbsetup_autobook.pdf

install-exec-local:
	$(mkinstalldirs) @prefix@/share/html
	$(mkinstalldirs) @prefix@/share/html/figures
	$(mkinstalldirs) @prefix@/share/pdfs
	$(INSTALL_DATA) $(FIGURES) @prefix@/share/html/figures
	$(CHAPTER2BOOK) -s @srcdir@/vmusbsetup.xml -b @srcdir@/vmusb_bookinfo.xml -o @builddir@
	$(INSTALL_DATA) $(BOOK) @prefix@/share/pdfs

clean-local:
	rm -f $(BOOK)

EXTRA_DIST=$(DOCS) $(FIGURES)
