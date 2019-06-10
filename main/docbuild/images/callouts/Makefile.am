
EXTRA_DIST = 10.eps 10.gif 10.pdf \
						 1.eps 1.gif 1.pdf \
						 2.eps 2.gif 2.pdf \
						 3.eps 3.gif 3.pdf \
						 4.eps 4.gif 4.pdf \
						 5.eps 5.gif 5.pdf \
						 6.eps 6.gif 6.pdf \
						 7.eps 7.gif 7.pdf \
						 8.eps 8.gif 8.pdf \
						 9.eps 9.gif 9.pdf 

install-exec-local: 
	$(mkinstalldirs) @prefix@/share/images/callouts
	$(INSTALL_DATA) @srcdir@/*.gif @prefix@/share/images;
