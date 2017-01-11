
sharedir = @prefix@/share/src/vmusb_simple_setup


share_DATA = CRawADCUnpacker.cpp \
	CRawADCUnpacker.h \
	CRawUnpacker.cpp \
	CRawUnpacker.h \
	MySpecTclApp.cpp \
	MySpecTclApp.h \
	SpecTclRC.tcl

installdirs : mkinstalldirs
	$(mkinstalldirs) $(sharedir)

install-data-local:
	$(INSTALL_DATA) Example_Makefile $(sharedir)/Makefile
