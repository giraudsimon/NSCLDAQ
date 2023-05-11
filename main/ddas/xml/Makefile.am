lib_LTLIBRARIES=libDSPParameters.la

COMMONCXXFLAGS=-I@srcdir@/tinyxml -I@top_srcdir@ @PIXIE_CPPFLAGS@ -g \
	-I@top_srcdir@/ddas/configuration \
	-DFIRMWARE_DEF_FILE=\"@prefix@/share/ddasreadout/DDASFirmwareVersions.txt\" \
	-DSOURCE_DIR=\"@srcdir@\" \
	-DSETFILE_TEMPLATE=\"@prefix@/share/ddasreadout/crate_1/crate_1.set\" -g

COMMONLDFLAGS = @top_builddir@/ddas/xml/tinyxml/libTinyXml2.la \
	@PIXIE_LDFLAGS@ \
	@top_builddir@/ddas/configuration/libConfiguration.la \
	@PLX_LDFLAGS@ -ldl


libDSPParameters_la_SOURCES=PixieSettingsReader.cpp SetFile.cpp \
	SetFileReader.cpp XMLSettingsWriter.cpp XMLSettingsReader.cpp \
	ModuleWriter.cpp SlotConfigReader.cpp CrateManager.cpp \
	CrateReader.cpp XMLCrateReader.cpp  PixieCrateReader.cpp \
	CrateWriter.cpp XMLCrateWriter.cpp PixieCrateWriter.cpp \
	SetFileCrateReader.cpp SetFileEditor.cpp \
	SetFileWriter.cpp SetFileCrateWriter.cpp \
	tinyxmlutil.cpp

libDSPParameters_la_CXXFLAGS=$(COMMONCXXFLAGS)
libDSPParameters_la_LIBADD=$(COMMONLDFLAGS)
libDSPParameters_la_LDFLAGS=-g

include_HEADERS=ModuleSettings.h SettingsReader.h PixieSettingsReader.h \
	SetFile.h SetFileReader.h SettingsWriter.h XMLSettingsWriter.h \
	XMLSettingsReader.h ModuleWriter.h SlotConfigReader.h \
	CrateManager.h CrateReader.h XMLCrateReader.h tinyxmlutil.h \
	PixieCrateReader.h CrateWriter.h XMLCrateWriter.h PixieCrateWriter.h \
	SetFileCrateReader.h SetFileEditor.h  \
	SetFileWriter.h SetFileCrateWriter.h \
	@srcdir@/tinyxml/tinyxml2.h

bin_PROGRAMS=toxml tocrate tosetfile

toxml_SOURCES=ToXmlMain.cpp toxml.h toxml.c ToUtils.h ToUtils.cpp
toxml_CXXFLAGS=$(COMMONCXXFLAGS)
toxml_LDADD=libDSPParameters.la $(COMMONLDFLAGS)
toxml_LDFLAGS=  -g

tocrate_SOURCES=ToCrateMain.cpp tocrate.h tocrate.c ToUtils.h ToUtils.cpp
tocrate_CXXFLAGS=$(COMMONCXXFLAGS)
tocrate_LDADD=libDSPParameters.la $(COMMONLDFLAGS)
tocrate_LDFLAGS=  -g

tosetfile_SOURCES=tosetfileMain.cpp tosetfile.h tosetfile.c ToUtils.cpp
tosetfile_CXXFLAGS=$(COMMONCXXFLAGS)
tosetfile_LDADD=libDSPParameters.la $(COMMONLDFLAGS) 
tosetfile_LDFLAGS=  -g


BUILT_SOURCES=toxml.c toxml.h tocrate.c tocrate.h tosetfile.c tosetfile.h
toxml.c: toxml.h

toxml.h: toxml.ggo
	$(GENGETOPT) -i@srcdir@/toxml.ggo -Ftoxml

tocrate.c: tocrate.h
tocrate.h: tocrate.ggo
	$(GENGETOPT) -i@srcdir@/tocrate.ggo -Ftocrate

tosetfile.c: tosetfile.h
tosetfile.h: tosetfile.ggo
	$(GENGETOPT) -i@srcdir@/tosetfile.ggo -Ftosetfile

noinst_PROGRAMS=unittests
unittests_SOURCES=TestRunner.cpp Asserts.h \
	setreadertests.cpp setfilereadertests.cpp \
	setcratereadertests.cpp xmlreadertests.cpp xmlcreadertests.cpp \
	xmlsetwritertests.cpp xmlcwritertests.cpp sedittests.cpp \
	setfilewritertests.cpp setfcwritetests.cpp \
	SetFileReader.cpp SetFile.cpp CrateReader.cpp SetFileCrateReader.cpp \
	XMLSettingsReader.cpp tinyxmlutil.cpp XMLCrateReader.cpp \
	XMLSettingsWriter.cpp XMLCrateWriter.cpp CrateWriter.cpp \
	SetFileEditor.cpp CrateManager.cpp \
	SetFileWriter.cpp SetFileCrateWriter.cpp

unittests_CXXFLAGS=@CPPUNIT_CFLAGS@ $(toxml_CXXFLAGS) -I@srcdir@/tinyxml \
	-DSRCDIR=\"@srcdir@\" @PIXIE_CPPFLAGS@ -g
unittests_LDFLAGS=-g   @CPPUNIT_LIBS@ \
	@top_builddir@/ddas/configuration/libConfiguration.la \
	@top_builddir@/ddas/xml/tinyxml/libTinyXml2.la	\
	@PIXIE_LDFLAGS@ \
	@PLX_LDFLAGS@ -ldl -g

## Requires in tree build.

TESTS=unittests                     # For now disable tests.

EXTRA_DIST=bimdev_Module_00.xml test.var crate_1.set \
	goodcrate.xml badcrate.xml README_toxxx.txt \
	README_libDSPParameters.txt tinyxml \
	tocrate.ggo toxml.ggo tosetfile.ggo
