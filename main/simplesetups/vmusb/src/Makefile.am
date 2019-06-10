
sharedir = @prefix@/share/src/vmusb_simple_setup


share_DATA = CRawADCUnpacker.cpp \
	CRawADCUnpacker.h \
	CRawUnpacker.cpp \
	CRawUnpacker.h \
	MySpecTclApp.cpp \
	MySpecTclApp.h \
	SpecTclRC.tcl


install-data-local:
	$(mkinstalldirs) $(sharedir)
	$(INSTALL_DATA) Example_Makefile $(sharedir)/Makefile

EXTRA_DIST=$(share_DATA)
