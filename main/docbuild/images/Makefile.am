
DIST_SUBDIRS = callouts

IMAGES = caution.eps caution.gif caution.pdf  \
				 home.eps home.gif home.pdf \
				 important.eps important.gif important.pdf \
				 next.eps next.gif next.pdf \
				 note.eps note.gif note.pdf \
				 prev.eps prev.gif prev.pdf \
				 tip.eps tip.gif tip.pdf \
				 toc-blank.eps toc-blank.gif toc-blank.pdf \
				 toc-minus.eps toc-minus.gif toc-minus.pdf \
				 toc-plus.eps toc-plus.gif toc-plus.pdf \
				 up.eps up.gif up.pdf \
				 warning.eps warning.gif warning.pdf

EXTRA_DIST = $(IMAGES) 


install-exec-local: 
	$(mkinstalldirs) @prefix@/share/images 
	$(INSTALL_DATA) @srcdir@/*.gif @prefix@/share/images;

